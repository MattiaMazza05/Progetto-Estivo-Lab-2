//includo librerie che mi servono in tutti i file
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

//macro per chiamata di sistema
#define SCALL_ERROR -1
#define SCALL(r, c, e) do{if((r = c) == SCALL_ERROR) {perror(e); exit(EXIT_FAILURE);}} while(0)
// macro per read 
#define SCALLREAD(r, loop_cond_op, read_loop_op, e) do {while((r = loop_cond_op)>0) {read_loop_op;} if(r== SCALL_ERROR){perror(e); exit(EXIT_FAILURE);}} while(0)

#define FINOAPROXDATO while (buffer[i] != '[' && i<bytesRead) {i++;} i++; //questa macro cicla fino a quanto non trova una [ ovvero l'inizio di un altro dato il secondo i++ mi serve per escludere la prossima [

#define ESTRAIDATO(nome_array) while (buffer[i] != ']' && i<bytesRead){nome_array[iC++] = buffer[i++];} //riempio l'array di supporto            

#define MAX_MSG_Q 10

#define PERROR_EXIT(messaggio) perror(messaggio); exit(1);
#define PERROR_BREAK(messaggio) perror(messaggio); break;

#define ESTRAI_EMERGENZA(campo) iC = 0; while(buffer[i] != ' ' && buffer[i] != '\n' && i < bytesRead) { campo[iC++] = buffer[i++]; } campo[iC] = '\0'; if (buffer[i] == ' ' || buffer[i] == '\n') i++; //questa macro mi permette di estrarre ogni singolo campo dall'eventuale file passato

#define ESTRAI_EMER_MAIN(campo) iC = 0; while(buffer[i] != ' ' && buffer[i] != '\0' ) { campo[iC++] = buffer[i++]; } campo[iC] = '\0'; if (buffer[i] == ' ' || buffer[i] == '\n') i++;

#define PRIORITY_0 0
#define PRIORITY_1 30
#define PRIORITY_2 10

#define DISTANZA_MANHATTAN(x1,y1,x2,y2, vel_socc) int ris1 = abs(x1-x2); int ris2 = abs(y1-y2); int vel = (ris1 + ris2)/vel_socc 

#define CAMBIO_STATO(stato_precedente, stato_successivo)for (int id = 0; id < mezzi_totali_necessari; id++){for (int j = 0; j < tot_mezzi; j++){if (mezzi_totali[j].id == mezzi_per_emer[id] && mezzi_totali[j].status == stato_precedente){mezzi_totali[j].status = stato_successivo;break;}}}