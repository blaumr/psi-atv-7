#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/*
 * Atividade - Produtor e Consumidor
 *
 * Thread padeiro:
 * - produz um pao a cada 1 segundo
 * - nao deixa a vitrine passar de 10 paes
 *
 * Thread cliente:
 * - compra um pao a cada 1,5 segundo
 * - espera caso nao haja paes disponiveis
 */

#define CAPACIDADE_VITRINE 10

volatile int saldo_vitrine = 0;

/* Mutex para proteger o acesso ao saldo da vitrine */
K_MUTEX_DEFINE(mutex_saldo);

/*
 * Semaforo contador.
 * Representa quantos paes estao disponiveis para compra.
 *
 * Valor inicial: 0
 * Valor maximo: 10
 */
K_SEM_DEFINE(paes_disponiveis, 0, CAPACIDADE_VITRINE);

/* -------- THREAD A: padeiro -------- */
void thread_padeiro(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    printk("Padeiro trabalhando\n");

    while (1) {
        k_msleep(1000);

        k_mutex_lock(&mutex_saldo, K_FOREVER);

        if (saldo_vitrine < CAPACIDADE_VITRINE) {
            saldo_vitrine++;

            printk("Padeiro produziu um pao\n");
            printk("Saldo vitrine: %d\n", saldo_vitrine);

            /*
             * Libera um pao para o cliente.
             * Ou seja, incrementa o semaforo contador.
             */
            k_sem_give(&paes_disponiveis);
        } else {
            printk("Vitrine cheia\n");
        }

        k_mutex_unlock(&mutex_saldo);
    }
}

/* -------- THREAD B: cliente -------- */
void thread_cliente(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    printk("Cliente aguardando paes\n");

    while (1) {
        k_msleep(1500);

        /*
         * Espera ate existir pelo menos um pao disponivel.
         * Se o semaforo estiver em zero, a thread bloqueia aqui.
         */
        k_sem_take(&paes_disponiveis, K_FOREVER);

        k_mutex_lock(&mutex_saldo, K_FOREVER);

        saldo_vitrine--;

        printk("Cliente comprou um pao\n");
        printk("Saldo vitrine: %d\n", saldo_vitrine);

        k_mutex_unlock(&mutex_saldo);
    }
}

/* Definicao das threads */
K_THREAD_DEFINE(threadPadeiro, 512, thread_padeiro,
                NULL, NULL, NULL,
                5, 0, 0);

K_THREAD_DEFINE(threadCliente, 512, thread_cliente,
                NULL, NULL, NULL,
                5, 0, 0);

/* -------- Funcao main -------- */
int main(void)
{
    while (1) {
        k_mutex_lock(&mutex_saldo, K_FOREVER);

        if (saldo_vitrine == 0) {
            printk("Sem paes no estoque\n");
        }

        k_mutex_unlock(&mutex_saldo);

        k_msleep(500);
    }

    return 0;
}
