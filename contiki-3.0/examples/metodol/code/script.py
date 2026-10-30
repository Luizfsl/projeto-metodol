import os
import subprocess
import re
import shutil
from concurrent.futures import ProcessPoolExecutor, as_completed

MAX_WORKERS = 2  # Execuções simultâneas na máquina

CONTIKI_DIR = "/home/user/contiki-3.0"
PROJECT_DIR = "/home/user/contiki-3.0/examples/metodol/code"
RESULTS_FOLDER = "/home/user/contiki-3.0/examples/metodol/resultados"

NODE_EXEC_FILE = os.path.join(PROJECT_DIR, "code_mote.c")
CSC_TEMPLATE = os.path.join(CONTIKI_DIR, "examples/metodol/simulation/simulation.csc")
TARGET = "sky"

# Modos de envio a comparar
TRANSMISSION_MODES = {
    "burst": 1,
    "batch": 2
}

# Quantidades de itens/pacotes
PACKET_RATES = {
    2: "2-items",
    4: "4-items",
    8: "8-items",
    16: "16-items",
    32: "32-items"
}

# Configurações de RDC
DUTY_CYCLE_SUITE = {
    "nullrdc": ("nullrdc_driver", 0),         # Rádio sempre ligado (baseline)
    "contikimac-16hz": ("contikimac_driver", 16) # ContikiMAC padrão
}

def patch_csc_with_prebuilt_firmware(csc_template_path, firmware_abs_path, output_csc_path):
    with open(csc_template_path, "r") as f:
        content = f.read()

    content = re.sub(
        r'<firmware[^>]*>.*?</firmware>',
        '<firmware EXPORT="copy">{}</firmware>'.format(firmware_abs_path),
        content
    )
    content = re.sub(r'<commands[^>]*>.*?</commands>', '', content)

    with open(output_csc_path, "w") as f:
        f.write(content)

def run_single_experiment_task(task_info):
    mode_name, mode_val, pkt_val, pkt_folder, rdc_name, driver, rate_hz = task_info

    task_id = "{}_{}_{}".format(mode_name, pkt_folder, rdc_name)

    work_dir = os.path.join(CONTIKI_DIR, "examples", "tmp_ws_{}".format(task_id))
    result_dir = os.path.join(RESULTS_FOLDER, mode_name, pkt_folder, rdc_name)
    os.makedirs(result_dir, exist_ok=True)
    os.makedirs(work_dir, exist_ok=True)

    print("[INÍCIO] Tarefa: {}".format(task_id))

    try:
        shutil.copy(NODE_EXEC_FILE, os.path.join(work_dir, "code_mote.c"))
        shutil.copy(os.path.join(PROJECT_DIR, "Makefile"), os.path.join(work_dir, "Makefile"))

        # Criação do project-conf.h
        with open(os.path.join(work_dir, "project-conf.h"), "w") as f:
            f.write("#ifndef PROJECT_CONF_H_\n#define PROJECT_CONF_H_\n\n")
            f.write("#define ENERGEST_CONF_ON 1\n")
            f.write("#define CC2420_CONF_AUTOACK 1\n\n")
            f.write("#undef NETSTACK_CONF_RDC\n#define NETSTACK_CONF_RDC {}\n\n".format(driver))
            f.write("#undef NETSTACK_CONF_MAC\n#define NETSTACK_CONF_MAC csma_driver\n\n")
            
            # Ajusta tamanho de buffer do pacote caso seja um Lote grande
            if mode_name == "batch":
                f.write("#undef UIP_CONF_BUFFER_SIZE\n#define UIP_CONF_BUFFER_SIZE 256\n\n")

            if driver == "contikimac_driver":
                f.write("#undef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE\n"
                        "#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE {}\n\n".format(rate_hz))
            f.write("#endif /* PROJECT_CONF_H_ */\n")

        env = os.environ.copy()
        env["JAVA_HOME"] = subprocess.check_output(
            ["bash", "-c", "dirname $(dirname $(readlink -f $(which java)))"],
            universal_newlines=True
        ).strip()

        # Clean
        subprocess.call(
            ["make", "TARGET={}".format(TARGET), "clean"],
            cwd=work_dir, env=env,
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
        )

        # Compilação passando flags MODE e PKT_COUNT
        compile_proc = subprocess.Popen(
            ["make", "TARGET={}".format(TARGET), "node",
             "MODE={}".format(mode_val), "PKT_COUNT={}".format(pkt_val),
             "CONTIKI={}".format(CONTIKI_DIR)],
            cwd=work_dir, env=env,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            universal_newlines=True
        )
        compile_out, _ = compile_proc.communicate()

        if compile_proc.returncode != 0:
            print("[ERRO] Falha na compilação: {}\n{}".format(task_id, compile_out))
            return task_id, False

        firmware_path = os.path.join(work_dir, "node.sky")
        if not os.path.exists(firmware_path):
            print("[ERRO] Firmware node.sky não encontrado: {}".format(task_id))
            return task_id, False

        csc_path = os.path.join(work_dir, "sim.csc")
        patch_csc_with_prebuilt_firmware(CSC_TEMPLATE, firmware_path, csc_path)

        num_runs = 2 # Defina aqui a quantidade de sementes/execuções por cenário
        for i in range(1, num_runs + 1):
            cooja_jar = os.path.join(CONTIKI_DIR, "tools/cooja/dist/cooja.jar")
            sim_proc = subprocess.Popen(
                ["java",
                 "-DJAVA_HOME={}".format(env["JAVA_HOME"]),
                 "-Xmx1024m",
                 "-jar", cooja_jar,
                 "-nogui={}".format(csc_path),
                 "-contiki={}".format(CONTIKI_DIR),
                 "-random-seed={}".format(__import__('random').randint(0, 2**31))],
                cwd=work_dir, env=env,
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                universal_newlines=True
            )

            log_path = os.path.join(result_dir, "log_run{}.txt".format(i))
            with open(log_path, "w") as lf:
                for line in sim_proc.stdout:
                    lf.write(line)
            sim_proc.wait()

            testlog = os.path.join(work_dir, "COOJA.testlog")
            if os.path.exists(testlog):
                shutil.copy(testlog, os.path.join(result_dir, "powertrace-run{}.txt".format(i)))
                os.remove(testlog)

        return task_id, True

    except Exception as e:
        print("[EXCEÇÃO] Tarefa {}: {}".format(task_id, e))
        return task_id, False

    finally:
        if os.path.exists(work_dir):
            shutil.rmtree(work_dir, ignore_errors=True)

if __name__ == "__main__":
    tasks = []
    for mode_name, mode_val in TRANSMISSION_MODES.items():
        for pkt_val, pkt_folder in PACKET_RATES.items():
            for rdc_name, (driver, rate_hz) in DUTY_CYCLE_SUITE.items():
                tasks.append((mode_name, mode_val, pkt_val, pkt_folder, rdc_name, driver, rate_hz))

    total_tasks = len(tasks)
    print("=" * 60)
    print(" INICIANDO ANÁLISE COMPARATIVA DE ENERGIA (BURST vs BATCH)")
    print(" Total de cenários: {}".format(total_tasks))
    print(" Execuções paralelas simultâneas: {}".format(MAX_WORKERS))
    print("=" * 60)

    completed_count = 0
    with ProcessPoolExecutor(max_workers=MAX_WORKERS) as executor:
        futures = {executor.submit(run_single_experiment_task, task): task for task in tasks}
        for future in as_completed(futures):
            task_id, success = future.result()
            completed_count += 1
            status = "✓" if success else "✗"
            print("--> {} {}: {}/{} cenários concluídos.".format(status, task_id, completed_count, total_tasks))

    print("=" * 60)
    print(" Todos os experimentos finalizados! Resultados salvos em:", RESULTS_FOLDER)
    print("=" * 60)
