# PROGRAMA
PROG = ajustePol gera_entrada

# Compilador
CC     = gcc
CFLAGS = -O3 -mavx -march=native -DLIKWID_PERFMON -I/usr/local/include 
LFLAGS = -lm -L/usr/local/lib -llikwid

# Lista de arquivos para distribuição.
DISTFILES = *.c *.h LEIAME* Makefile 
DISTDIR = `basename ${PWD}`

.PHONY: all clean purge dist

all: $(PROG)

ajustePol: ajustePol.c utils.c
	$(CC) -o $@ $(CFLAGS) $^ $(LFLAGS)

gera_entrada: gera_entrada.c
	$(CC) -o $@ $(CFLAGS) $^ -lm

clean:
	@echo "Limpando sujeira ..."
	@rm -f *~ *.bak core

purge: clean
	@echo "Limpando tudo ..."
	@rm -f $(PROG) *.o a.out $(DISTDIR) $(DISTDIR).tar

dist: purge
	@echo "Gerando arquivo de distribuição ($(DISTDIR).tar) ..."
	@ln -s . $(DISTDIR)
	@tar -cvf $(DISTDIR).tar $(addprefix ./$(DISTDIR)/, $(DISTFILES))
	@rm -f $(DISTDIR)

# # PROGRAMA
#     PROG = ajustePol gera_entrada

# # Compilador
#     CC     = gcc
#     CFLAGS = -O3 -DLIKWID_PERFMON -I/usr/local/include 
#     LFLAGS = -lm -L/usr/local/lib -llikwid

# # Lista de arquivos para distribuição.
# # LEMBRE-SE DE ACRESCENTAR OS ARQUIVOS ADICIONAIS SOLICITADOS NO ENUNCIADO DO TRABALHO
# DISTFILES = *.c *.h LEIAME* Makefile 
# DISTDIR = `basename ${PWD}`

# .PHONY: all clean purge dist

# ajustePol: ajustePol.c utils.c
# 	gcc -o $@ $(CFLAGS) $^ $(LFLAGS)

# gera_entrada: gera_entrada.c
# 	gcc -o $@ $^ -lm

# clean:
# 	@echo "Limpando sujeira ..."
# 	@rm -f *~ *.bak core 

# purge:  clean
# 	@echo "Limpando tudo ..."
# 	@rm -f $(PROG) *.o a.out $(DISTDIR) $(DISTDIR).tar

# dist: purge
# 	@echo "Gerando arquivo de distribuição ($(DISTDIR).tar) ..."
# 	@ln -s . $(DISTDIR)
# 	@tar -cvf $(DISTDIR).tar $(addprefix ./$(DISTDIR)/, $(DISTFILES))
# 	@rm -f $(DISTDIR)
