
# TODO

 - aggiorna LVGL anziche' usare la tua fork

# Note

 - Il primo led in alto indica lo stato dello slave (per il master) e del master (per lo slave)
 - Sto provando a usare Kconfig per configurare LVGL, e temo sia ancora un po' acerbo. Posso impostare solo `void *` come tipo per il campo `user_data`; quando creo un oggetto usando una copia `user_data` viene copiato, per cui mi trovo con due oggetti che puntano alla stessa cosa che viene quindi liberata due volte (corruzione di memoria). Devo assolutamente cercare di risolvere il problema; nel frattempo ricordati di registrare tutti i callback alla fine della funzione `open`.