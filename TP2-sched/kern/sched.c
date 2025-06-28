#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

#define PRIORIDAD_MIN 1
#define PRIORIDAD_MAX 10

struct SchedStats {
	int sched_calls;
	int env_executed;
	int time;
};

static struct SchedStats sched_stats;

void sched_halt(void);
void actualizar_stats_proceso(struct Env *e);
static struct Env *last_env = NULL;
void print_statistics(void);

void
actualizar_stats_proceso(struct Env *e)
{
	e->env_count++;
	if (e->env_runs == 0) {
		sched_stats.env_executed++;
		e->env_start_time = sched_stats.time;
	}
	e->env_runs++;
}

void
sched_prioridades()
{
	int prioridad_actual = PRIORIDAD_MAX + 1;
	struct Env *elegido = NULL;

	int next_curenv_ind = 0;
	if (curenv) {
		next_curenv_ind = ENVX(curenv->env_id) + 1;
	}

	// Actualizar el tiempo de los stats
	sched_stats.time++;
	if (last_env && last_env != curenv && last_env->env_status != ENV_RUNNING) {
		last_env->env_finish_time = sched_stats.time;
	}

	for (int i = 0; i < NENV; i++) {
		int ind = (i + next_curenv_ind) % NENV;
		if (envs[ind].env_status == ENV_RUNNABLE) {
			if (envs[ind].env_priority < prioridad_actual) {
				elegido = &envs[ind];
				prioridad_actual = envs[ind].env_priority;
			}
		}
	}

	if (!elegido && curenv && curenv->env_status == ENV_RUNNING &&
	    curenv->env_cpunum == thiscpu->cpu_id) {
		last_env = curenv;
		env_run(curenv);
		return;
	}

	if (elegido) {
		if (elegido->env_priority == PRIORIDAD_MAX) {
			for (int i = 0; i < NENV; i++) {
				if (envs[i].env_status == ENV_RUNNABLE) {
					envs[i].env_priority = PRIORIDAD_MIN;
				}
			}
		} else if (elegido->env_runs > 0) {
			elegido->env_priority++;
		}
		// Actualizar estadísticas por proceso
		actualizar_stats_proceso(elegido);

		last_env = elegido;
		env_run(elegido);
	}

	sched_halt();
}

// Choose a user environment to run and run it.
void
sched_yield(void)
{
	sched_stats.sched_calls++;
	sched_stats.time++;
#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running. Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here - Round robin

	if (last_env && last_env != curenv && last_env->env_status != ENV_RUNNING) {
		last_env->env_finish_time = sched_stats.time;
	}

	int env_anterior = (curenv ? curenv - envs : 0) % NENV;
	for (int i = 0; i < NENV; i++) {
		int indice = (env_anterior + i) % NENV;
		if (envs[indice].env_status == ENV_RUNNABLE) {
			last_env = &envs[indice];
			env_run(&envs[indice]);
			return;
		}
	}
	if (curenv && curenv->env_status == ENV_RUNNING) {
		last_env = curenv;
		env_run(curenv);
		return;
	} else {
		last_env = NULL;
		sched_halt();
	}

#endif

#ifdef SCHED_PRIORITIES
	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.
	//
	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.

	// Your code here - Priorities
	// puse numeros arbitrarios a todo dsp ver q onda
	sched_prioridades();


#endif

	// Without scheduler, keep runing the last environment while it exists
	if (curenv) {
		last_env = curenv;
		env_run(curenv);
	}

	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
#ifdef SCHED_PRIORITIES
		print_statistics();
#endif
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	last_env = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}

void
print_statistics()
{
	cprintf("-----Estadísticas del scheduler-----\n");
	cprintf("Total sched calls: %d\n", sched_stats.sched_calls);
	cprintf("Procesos ejecutados: %d\n", sched_stats.env_executed);
	cprintf("Procesos que se ejecutaron:\n");
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_runs > 0) {
			cprintf("Env %08x: times_chosen=%d, start=%d, "
			        "finish=%d, runs=%d\n",
			        envs[i].env_id,
			        envs[i].env_count,
			        envs[i].env_start_time,
			        envs[i].env_finish_time,
			        envs[i].env_runs);
		}
	}
}
