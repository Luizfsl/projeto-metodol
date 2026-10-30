CONTIKI_PROJECT = node
all: $(CONTIKI_PROJECT)

# Habilita ferramenta de medição energética
APPS += powertrace

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

# Passa o modo de transmissão e tamanho para o C
ifdef MODE
CFLAGS += -DTRANSMISSION_MODE=$(MODE)
endif

ifdef PKT_COUNT
CFLAGS += -DPACKET_COUNT=$(PKT_COUNT)
endif

CONTIKI = ../..
CONTIKI_WITH_RIME = 1

include $(CONTIKI)/Makefile.include
