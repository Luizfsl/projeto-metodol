# projeto-metodol

Repositório referente ao projeto desenvolvido na disciplina de METODOL EXPRESSAO TEC-CIENTIFICA

## Configuração do ambiente

Basicamente a pasta contiki-3.0/ estará dentro da vm ubuntu 14.04 e dentro dela que faremos as nossas modificações;

É importante ter e manter a pasta atualizada no git a fim de manter o mesmo repositório modificado.

```text
/home/user/contiki-3.0/
└── examples/
    └── metodol/
        ├── code/                  # Diretório principal de código e automação
        │   ├── code_node.c        # Aplicação C (lógica Unicast/Powertrace)
        │   ├── Makefile           # Instruções de compilação
        │   ├── project-conf.h     # Parâmetros da pilha MAC/RDC e rede
        │   └── script.py          # Script de automação e execução paralela
        │
        ├── simulation/            # Arquivos de cenário do Cooja
        │   └── simulation.csc     # Arquivo base de simulação (com ScriptRunner)
        │
        └── resultados/            # Diretório gerado para armazenamento dos logs
            ├── burst/             # Logs de consumo energético do modo Rajada
            └── batch/             # Logs de consumo energético do modo Lote
