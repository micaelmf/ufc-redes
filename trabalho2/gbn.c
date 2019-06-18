#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ******************************************************************
 ALTERNANDO BIT E EMULADOR DE REDE GO-BACK-N: VERSÃO 1.1 J.F.Kurose
   
   Este código deve ser usado para PA2, unidirecional ou bidirecional
   protocolos de transferência de dados (de A para B. Transferência bidirecional de dados
   é para crédito extra e não é necessário). Propriedades de rede:
   - uma média de atrasos de rede de sentido único, cinco unidades de tempo
       são outras mensagens no canal para GBN), mas podem ser maiores
   - pacotes podem ser corrompidos (o cabeçalho ou a porção de dados)
       ou perdido, de acordo com as probabilidades definidas pelo usuário
   - os pacotes serão entregues na ordem em que foram enviados
       (embora alguns possam ser perdidos).
**********************************************************************/

#define BIDIRECTIONAL 0    /* mude para 1 se quiser tornar a rede bidirecional */
                           /* e escreva a rotina B_output */

/* uma "msg" é uma unidade de dados passada da aplicação(camada 5, código feito) para*/
/* camada 4 (código a ser feito).  Ela contém os dados (caracteres) a serem entregues */
/* para a outra camada 5 via protocolo de transporte a ser implementado*/
struct msg
{
    char data[20];
};

/* um pacote é uma unidade de dados passado da camada 4 (código a ser implementado) para camada */
/* 3 (já implementada).  A struct pacote deve ser utilizada está abaixo*/
struct pkt
{
    int seqnum;
    int acknum;
    int checksum;
    char payload[20];
};

void starttimer(int AorB, float increment);
void stoptimer(int AorB);
void tolayer3(int AorB, struct pkt packet);
void tolayer5(int AorB, char datasent[20]);

/********* PARA O TRABALHO PRECISA ESCREVER AS PRÓXIMAS 6 ROTINAS *********/

/* Definição de protótipos de funções a serem implementadas */
/* mais abaixo, no código provido.*/

#define BUFSIZE 64

struct Sender
{
    int base;
    int nextseq;
    int window_size;
    float estimated_rtt;
    int buffer_next;
    struct pkt packet_buffer[BUFSIZE];
} A;

struct Receiver
{
    int expect_seq;
    struct pkt packet_to_send;
} B;

int get_checksum(struct pkt *packet)
{
    int checksum = 0;
    checksum += packet->seqnum;
    checksum += packet->acknum;
    for (int i = 0; i < 20; ++i)
        checksum += packet->payload[i];
    return checksum;
}

void send_window(void)
{
    while (A.nextseq < A.buffer_next && A.nextseq < A.base + A.window_size)
    {
        struct pkt *packet = &A.packet_buffer[A.nextseq % BUFSIZE];
        printf("  send_window: enviar pacote (seq=%d): %s\n", packet->seqnum, packet->payload);
        tolayer3(0, *packet);
        if (A.base == A.nextseq)
            starttimer(0, A.estimated_rtt); // inicia o temporizador
        ++A.nextseq;
    }
}

/* Chamada da camada 3, quando pacotes chegama para camada 4 */
void A_output(struct msg message)
{
    if (A.buffer_next - A.base >= BUFSIZE)
    {
        printf("  A_output: buffer cheio. descartar mensagem: %s\n", message.data);
        return;
    }
    //|B|o|o|o|o|o|o|o|o|o|o |o | N
    //|0|1|2|3|4|5|6|7|8|9|10|11|
    printf("  A_output: guardar pacote (seq=%d): %s\n", A.buffer_next, message.data); // mensagem na posição buffer_next
    struct pkt *packet = &A.packet_buffer[A.buffer_next % BUFSIZE]; //packet guadar o endereço do buffer na posição buffer_next
    packet->seqnum = A.buffer_next; // packet acessa seqnum atribuindo buffer_next
    memmove(packet->payload, message.data, 20); // encapsula o pacote
    packet->checksum = get_checksum(packet);
    ++A.buffer_next;
    send_window();
}

/* need be completed only for extra credit */
void B_output(struct msg message)
{
    printf("  B_output: uni-direcional. ignorar.\n");
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    if (packet.checksum != get_checksum(&packet))
    {
        printf("  A_input: pacote corrumpido. descartar.\n");
        return;
    }
    if (packet.acknum < A.base)
    {
        printf("  A_input: recebeu o NAK (ack=%d). descartar.\n", packet.acknum);
        return;
    }
    printf("  A_input: recebeu ACK (ack=%d)\n", packet.acknum);
    A.base = packet.acknum + 1;
    if (A.base == A.nextseq)
    {
        stoptimer(0);
        printf("  A_input: para o tempo\n");
        send_window();
    }
    else
    {
        starttimer(0, A.estimated_rtt);
        printf("  A_input: tempo + %f\n", A.estimated_rtt);
    }
}

/* chamado quando temporizador de A estoura */
void A_timerinterrupt(void)
{
    for (int i = A.base; i < A.nextseq; ++i)
    {
        struct pkt *packet = &A.packet_buffer[i % BUFSIZE];
        printf("  A_timerinterrupt: resend packet (seq=%d): %s\n", packet->seqnum, packet->payload);
        tolayer3(0, *packet);
    }
    starttimer(0, A.estimated_rtt);
    printf("  A_timerinterrupt: timer + %f\n", A.estimated_rtt);
}

/* Será chamada uma única vez antes de qualquer outra rotina de A seja chamada */
/* Você pode utilizar para qualquer inicialização */
void A_init(void)
{
    A.base = 1;
    A.nextseq = 1;
    A.window_size = 8;
    A.estimated_rtt = 15;
    A.buffer_next = 1;
}
/* Como a transferência é simplex de A-para-B, não há B_output() */

/* chamada da camada 3, quando um pacote chega para camada 4 em B*/
void B_input(struct pkt packet)
{
    if (packet.checksum != get_checksum(&packet))
    {
        printf("  B_input: pacote corrumpido. enviar NAK (ack=%d)\n", B.packet_to_send.acknum);
        tolayer3(1, B.packet_to_send);
        return;
    }
    if (packet.seqnum != B.expect_seq)
    {
        printf("  B_input: nao e o numero de sequencia esperado. enviar NAK (ack=%d)\n", B.packet_to_send.acknum);
        tolayer3(1, B.packet_to_send);
        return;
    }

    printf("  B_input: recuperar pacote (recv) (seq=%d): %s\n", packet.seqnum, packet.payload);
    tolayer5(1, packet.payload); // envia o pacote recebido para a camada de aplicação

    printf("  B_input: enviar ACK (send) (ack=%d)\n", B.expect_seq);
    B.packet_to_send.acknum = B.expect_seq; // atribui um numero para ACK igual ao numero experado da sequencia
    B.packet_to_send.checksum = get_checksum(&B.packet_to_send); //atribuir numero de verificação checksum
    tolayer3(1, B.packet_to_send);

    ++B.expect_seq; //pre incrementa o numero de sequencia esperado
}

/* chamada quando temporizador de B estoura */
void B_timerinterrupt(void)
{
    printf("  B_timerinterrupt: B doesn't have a timer. ignore.\n");
}

/* Será chamada uma única vez antes de qualquer outra rotina de B seja chamada */
/* Você pode utilizar para qualquer inicialização */
void B_init(void)
{
    B.expect_seq = 1;
    B.packet_to_send.seqnum = -1;
    B.packet_to_send.acknum = 0;
    memset(B.packet_to_send.payload, 0, 20);
    B.packet_to_send.checksum = get_checksum(&B.packet_to_send);
}

/*****************************************************************
***************** EMULADOR DE REDE INICIA ABAIXO ***********
o código emula da camada 3 p/ baixo:
  - emula a transmissão e entrega, com possibilidade de corrupção de bits
    e perda de pacote, de pacotes através das interfaces 3/4
  - manipula o início/parada de um temporizador, e gera interrupção deles, o que resulta
    na chamada aos manipuladores de tempo que serão implementados
  - gera mensagem a ser enviada, passada da camada 5 p/ 4

NÃO HÁ NECESSIDADE DE LER OU ENTENDER O CÓDIGO ABAIXO PARA RESOLVER O TRABALHO. 
VOCÊ NÃO DEVE TOCAR OU REFERENCIAR QUALQUER STRUCT ABAIXO. 
Se tiver interessado em entender, tudo bem. Mas, não modifique!
******************************************************************/

struct event {
   float evtime;           /* evento tempo */
   int evtype;             /* código do tipo de evento */
   int eventity;           /* entidade onde ocorre o evento */
   struct pkt *pktptr;     /* ptr p/ pacote associado, se houver, com evento */
   struct event *prev;
   struct event *next;
 };
struct event *evlist = NULL;   /* a lista de evento */

/* eventos possíveis: */
#define  TIMER_INTERRUPT 0
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2

#define  OFF             0
#define  ON              1
#define   A    0
#define   B    1



int TRACE = 1;             /* p/ debbug */
int nsim = 0;              /* número de msgs da camada 5 p/ 4 */
int nsimmax = 0;           /* número de msgs a gerar, então para */
float time = (float)0.000;
float lossprob;            /* probabilidade de um pacote ser descartado  */
float corruptprob;         /* probabilidade de um bit no pacote ser trocado */
float lambda;              /* taxa de chegada de msgs da camada 5 */
int   ntolayer3;           /* número enviado p/ camada 3 */
int   nlost;               /* número perdido em média */
int ncorrupt;              /* número corrompido em média */

void main()
{
   struct event *eventptr;
   struct msg  msg2give;
   struct pkt  pkt2give;

   int i,j;
   /* char c; // variável local não referenciada, removida */

   init();
   A_init();
   B_init();

   while (1) {
        eventptr = evlist;            /* pega prócimo evento p/ simular */
        if (eventptr==NULL)
           goto terminate;
        evlist = evlist->next;        /* remove o evento da lista */
        if (evlist!=NULL)
           evlist->prev=NULL;
        if (TRACE>=2) {
           printf("\nEVENT tempo: %f,",eventptr->evtime);
           printf("  tipo: %d",eventptr->evtype);
           if (eventptr->evtype==0)
	       printf(", parada de temporizador  ");
             else if (eventptr->evtype==1)
               printf(", da camada 5 ");
             else
	     printf(", da camada 3 ");
           printf(" entidade: %d\n",eventptr->eventity);
           }
        time = eventptr->evtime;        /* atualiza tempo p/ próximo evento */
        if (nsim==nsimmax)
	  break;                        /* tudo realizado na simulação */
        if (eventptr->evtype == FROM_LAYER5 ) {
            generate_next_arrival();   /* define a chegada futura */
            /* preenche msg com string de mesmo caractere */
            j = nsim % 26;
            for (i=0; i<20; i++)
               msg2give.data[i] = 97 + j;
            if (TRACE>2) {
               printf("          MAINLOOP: dado entregue ao estudante: ");
                 for (i=0; i<20; i++)
                  printf("%c", msg2give.data[i]);
               printf("\n");
	     }
            nsim++;
            if (eventptr->eventity == A)
               A_output(msg2give);
             else
               B_output(msg2give);
            }
          else if (eventptr->evtype ==  FROM_LAYER3) {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i=0; i<20; i++)
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
	    if (eventptr->eventity ==A)      /* entrega o pacote */
   	       A_input(pkt2give);            /* chamando entidade apropriada */
            else
   	       B_input(pkt2give);
	    free(eventptr->pktptr);          /* libera a memória do pacote */
            }
          else if (eventptr->evtype ==  TIMER_INTERRUPT) {
            if (eventptr->eventity == A)
	       A_timerinterrupt();
             else
	       B_timerinterrupt();
             }
          else  {
	     printf("ERRO INTERNO: tipo de envento desconhecido \n");
             }
        free(eventptr);
        }

terminate:
   printf(" Simulador terminou em tempo %f\n depois de enviar %d msgs da camada 5\n",time,nsim);
}



void init()                         /* inicializa simulador */
{
  int i;
  float sum, avg;
  float jimsrand();


   printf("-----  Simulador Para e Espera - Version 1.1 -------- \n\n");
   printf("Informe a quantidade de mensagem a simular: ");
   scanf("%d",&nsimmax);
   printf("Informe a probabilidade de perda de pacote [informe 0.0 sem perdas]:");
   scanf("%f",&lossprob);
   printf("Inform a probabilidade de corromper pacote [informe 0.0 sem corromper]:");
   scanf("%f",&corruptprob);
   printf("Informe o tempo entre msgs do emissor na camada 5[ > 0.0]:");
   scanf("%f",&lambda);
   printf("Informe TRACE:");// >2 p/ mensagens de debug
   scanf("%d",&TRACE);

   srand(9999);              /* inicia gerador de números aleatórios */
   sum = (float)0.0;         /* testa números aleatórios gerados para o código */
   for (i=0; i<1000; i++)
      sum=sum+jimsrand();    /* jimsrand() deve ser uniforme em [0,1] */
   avg = sum/(float)1000.0;
   if (avg < 0.25 || avg > 0.75) {
    printf("Provavel que a geração de números aleatórios na sua máquina\n" );
    printf("seja diferente que a esperada pelo emulador. Por favor olhe\n");
    printf("a rotina jimsrand() no código do emulador. Desculpe. \n");
    exit(0);
    }

   ntolayer3 = 0;
   nlost = 0;
   ncorrupt = 0;

   time=(float)0.0;                    /* inicia tempo em 0.0 */
   generate_next_arrival();     /* inicia lista de eventos */
}

/****************************************************************************/
/* jimsrand(): retorna float entre [0,1].  Rotina abaixo é utilizada */
/* para isolar toda geração aleatória de número em um lugar. Assumimos que*/
/* a função rand() retorna um int entre [0,mmm]        */
/****************************************************************************/
float jimsrand()
{
  double mmm = RAND_MAX;   /* maior int  - DEPENDENTE DE MÁQUINA!!!!!!!!   */
  float x;                   /* alguns estudantes talvez precisem modificar mmm*/
  x = (float)(rand()/mmm);            /* x deve ser uniforme entre [0,1] */
  return(x);
}

/********************* ROTINAS DE MANIPULAÇÃO DE EVENTOS *******/
/*  O pŕoximo conjunto de rotinas manipula a lista de eventos  */
/*****************************************************/

void generate_next_arrival()
{
   double x,log(),ceil();
   struct event *evptr;
   /* char *malloc(); // redefinição de malloc removida */
   /* float ttime; // variável local não referenciada removida*/
   /* int tempint; // variável local não referenciada removida*/

   if (TRACE>2)
       printf("          GERA PRÓXIMA CHEGADA: cria nova chegada\n");

   x = lambda*jimsrand()*2;  /* x é uniforme entre [0,2*lambda] */
                             /* tendo média de lambda        */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  (float)(time + x);
   evptr->evtype =  FROM_LAYER5;
   if (BIDIRECTIONAL && (jimsrand()>0.5) )
      evptr->eventity = B;
    else
      evptr->eventity = A;
   insertevent(evptr);
}


void insertevent(p)
   struct event *p;
{
   struct event *q,*qold;

   if (TRACE>2) {
      printf("            INSERE EVENTO: tempo é %lf\n",time);
      printf("            INSERE EVENTO: tempo futuro será %lf\n",p->evtime);
      }
   q = evlist;     /* q aponta p/ cabeça da lista em que struct p é inserida */
   if (q==NULL) {   /* lista vazia */
        evlist=p;
        p->next=NULL;
        p->prev=NULL;
        }
     else {
        for (qold = q; q !=NULL && p->evtime > q->evtime; q=q->next)
              qold=q;
        if (q==NULL) {   /* fim da lista */
             qold->next = p;
             p->prev = qold;
             p->next = NULL;
             }
           else if (q==evlist) { /* cabeça da lista */
             p->next=evlist;
             p->prev=NULL;
             p->next->prev=p;
             evlist = p;
             }
           else {     /* meio da lista */
             p->next=q;
             p->prev=q->prev;
             q->prev->next=p;
             q->prev=p;
             }
         }
}

void printevlist()
{
  struct event *q;
  /* int i; // variável local não referenciada removida*/
  printf("--------------\nLista de evento segue:\n");
  for(q = evlist; q!=NULL; q=q->next) {
    printf("Tempo evento: %f, tipo: %d entidade: %d\n",q->evtime,q->evtype,q->eventity);
    }
  printf("--------------\n");
}



/********************** ROTINAS QUE PODEM SER CHAMADAS ***********************/

/* chamada p/ cancelar um temporizador já iniciado */
void stoptimer(AorB)
int AorB;  /* A or B está tentando parar o temporizador */
{
 struct event *q;/* ,*qold; // variável local não referenciada removida*/

 if (TRACE>2)
    printf("          PARA TEMPORIZADOR: parando temporizador em %f\n",time);
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
 for (q=evlist; q!=NULL ; q = q->next)
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) {
       /* remove este evento */
       if (q->next==NULL && q->prev==NULL)
             evlist=NULL;         /* remove o primeiro e único evento da lista*/
          else if (q->next==NULL) /* fim da lista - há um na frente */
             q->prev->next = NULL;
          else if (q==evlist) { /* início da lista - deve haver evento depois*/
             q->next->prev=NULL;
             evlist = q->next;
             }
           else {     /* meio da lista */
             q->next->prev = q->prev;
             q->prev->next =  q->next;
             }
       free(q);
       return;
     }
  printf("Warning: incapaz de cancelar seu temporizador. Não estava rodando.\n");
}


void starttimer(AorB,increment)
int AorB;  /* A ou B está tentando parar o temporizador */
float increment;
{

 struct event *q;
 struct event *evptr;
 /* char *malloc(); // redefinição de malloc removida*/

 if (TRACE>2)
    printf("          INICIA TEMPORIZADOR: inicia temporizador em %f\n",time);
 /* verifica se o temporizador já iniciou, se sim, então avisa */
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
   for (q=evlist; q!=NULL ; q = q->next)
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) {
      printf("Warning: tentativa de iniciar temporizador já iniciado\n");
      return;
      }

/* cria um evento futuro p/ quando temporizador estourar*/
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  (float)(time + increment);
   evptr->evtype =  TIMER_INTERRUPT;
   evptr->eventity = AorB;
   insertevent(evptr);
}


/************************** PARA CAMADA 3 ***************/
void tolayer3(AorB,packet)
int AorB;  /* A ou B está tentando parar o temporizador */
struct pkt packet;
{
 struct pkt *mypktptr;
 struct event *evptr,*q;
 /* char *malloc(); // redefinição de malloc removida */
 float lastime, x, jimsrand();
 int i;


 ntolayer3++;

 /* simula perda: */
 if (jimsrand() < lossprob)  {
      nlost++;
      if (TRACE>0)
	printf("          PARA CAMADA 3: pacote sendo perdido\n");
      return;
    }

/* faz uma cópia do pacote para que o código continue com ela caso*/
/* código implementado descarte*/
 mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
 mypktptr->seqnum = packet.seqnum;
 mypktptr->acknum = packet.acknum;
 mypktptr->checksum = packet.checksum;
 for (i=0; i<20; i++)
    mypktptr->payload[i] = packet.payload[i];
 if (TRACE>2)  {
   printf("          PARA CAMADA 3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
	  mypktptr->acknum,  mypktptr->checksum);
    for (i=0; i<20; i++)
        printf("%c",mypktptr->payload[i]);
    printf("\n");
   }

/* cria evento futuro para chegada de pacote no outro lado */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtype =  FROM_LAYER3;   /* pacote será retirado da camada 3 */
  evptr->eventity = (AorB+1) % 2; /* evento ocorre na outra entidade */
  evptr->pktptr = mypktptr;       /* salva ponteiro para cópia do pacote */
/* finalmente, computa o tempo de chegada do pacote no outro host.
   o meio não reordena, garante que o pacote chega entre 1 e 10
   unidades de tempo após a última chegada de pacote*/
 lastime = time;
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
 for (q=evlist; q!=NULL ; q = q->next)
    if ( (q->evtype==FROM_LAYER3  && q->eventity==evptr->eventity) )
      lastime = q->evtime;
 evptr->evtime =  lastime + 1 + 9*jimsrand();



 /* simula corrupção: */
 if (jimsrand() < corruptprob)  {
    ncorrupt++;
    if ( (x = jimsrand()) < .75)
       mypktptr->payload[0]='Z';   /* corrompe payload */
      else if (x < .875)
       mypktptr->seqnum = 999999;
      else
       mypktptr->acknum = 999999;
    if (TRACE>0)
	printf("          PARA CAMADA 3: pacote sendo corrompido\n");
    }

  if (TRACE>2)
     printf("          PARA CAMADA 3: escalonando chegada no outro lado\n");
  insertevent(evptr);
}

void tolayer5(AorB,datasent)
  int AorB;
  char datasent[20];
{
  int i;
  if (TRACE>2) {
     printf("          PARA CAMADA 5: dado recebido: ");
     for (i=0; i<20; i++)
        printf("%c",datasent[i]);
     printf("\n");
   }

}