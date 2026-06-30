#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define LIMITE_ESTOQUE      10
#define PILHA_PRODUTOR      512
#define PILHA_CONSUMIDOR    512
#define PRIORIDADE_THREADS  5

static volatile int quantidade_paes = 0;

K_MUTEX_DEFINE(controle_estoque);
K_SEM_DEFINE(sem_paes, 0, LIMITE_ESTOQUE);

static void produtor(void *a, void *b, void *c)
{
    ARG_UNUSED(a);
    ARG_UNUSED(b);
    ARG_UNUSED(c);

    printk("Produtor iniciado\n");

    while (1) {
        k_msleep(1000);

        k_mutex_lock(&controle_estoque, K_FOREVER);

        if (quantidade_paes < LIMITE_ESTOQUE) {
            quantidade_paes++;

            printk("Pao produzido\n");
            printk("Quantidade atual: %d\n", quantidade_paes);

            k_sem_give(&sem_paes);
        } else {
            printk("Estoque cheio\n");
        }

        k_mutex_unlock(&controle_estoque);
    }
}

static void consumidor(void *a, void *b, void *c)
{
    ARG_UNUSED(a);
    ARG_UNUSED(b);
    ARG_UNUSED(c);

    printk("Consumidor iniciado\n");

    while (1) {
        k_msleep(1500);

        k_sem_take(&sem_paes, K_FOREVER);

        k_mutex_lock(&controle_estoque, K_FOREVER);

        if (quantidade_paes > 0) {
            quantidade_paes--;

            printk("Pao consumido\n");
            printk("Quantidade atual: %d\n", quantidade_paes);
        }

        k_mutex_unlock(&controle_estoque);
    }
}

K_THREAD_DEFINE(id_produtor,
                PILHA_PRODUTOR,
                produtor,
                NULL, NULL, NULL,
                PRIORIDADE_THREADS,
                0,
                0);

K_THREAD_DEFINE(id_consumidor,
                PILHA_CONSUMIDOR,
                consumidor,
                NULL, NULL, NULL,
                PRIORIDADE_THREADS,
                0,
                0);

int main(void)
{
    printk("Sistema produtor-consumidor iniciado\n");

    while (1) {
        k_mutex_lock(&controle_estoque, K_FOREVER);

        if (quantidade_paes == 0) {
            printk("Estoque vazio\n");
        }

        k_mutex_unlock(&controle_estoque);

        k_msleep(500);
    }

    return 0;
}