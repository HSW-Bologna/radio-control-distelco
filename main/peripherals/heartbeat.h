#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__

/*
 *  Inizializza il task che fa lampeggiare il led run.
 *
 *  priority: priorita' del task di gestione del led.
 */
void heartbeat_init(int priority);

#endif