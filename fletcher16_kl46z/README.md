# Fletcher-16 na FRDM-KL46Z

Contido:
- Makefile.fletcher   -> vai xunto ao Makefile da práctica 4 (onde está a carpeta SDK/)
- fletcher/           -> fletcher16.c/.h, fletcher_main.c (benchmark), test_host.c

## Pasos (dentro de ~/SE/p2/"practica4 (3)"/practica4)
1. Copia Makefile.fletcher e a carpeta fletcher/ aquí.
2. Proba o algoritmo no PC:      make -f Makefile.fletcher test      (debe dicir TODO OK)
3. Placa conectada a WSL (usbipd attach --wsl --busid 2-1).
4. Terminal serie nunha segunda xanela (a saída sae pola UART virtual de OpenSDA):
       screen /dev/ttyACM0 115200
   (Sae con Ctrl+A, K, Y.)
5. Grava e executa, unha variante cada vez:
       make -f Makefile.fletcher BUILD=O0    N=100  flash
       make -f Makefile.fletcher BUILD=Ofast N=100  flash
       make -f Makefile.fletcher BUILD=O0    N=1024 flash
       make -f Makefile.fletcher BUILD=Ofast N=1024 flash
   Tras gravar, a placa reiníciase e imprime os resultados (se non ves nada,
   preme o botón RESET da placa co terminal xa aberto).
