#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

/*
 *  Inizializza il modulo VSPI condiviso tra i vari circuiti integrati
 */
void system_spi_init(void);

void system_spi_take(void);
void system_spi_give(void);

#endif