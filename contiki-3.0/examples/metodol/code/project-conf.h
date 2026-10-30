#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/* Tamanho da fila de pacotes do rádio */
#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM 16

/* Mantém o rádio sempre ativo */
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC nullrdc_driver

#endif /* PROJECT_CONF_H_ */
