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

/********* ALUNOS ESCREVER AS PRÓXIMAS SETE ROTINAS *********/

enum SenderState //conjunto de valores inteiros representados por identificadores
{
    WAIT_LAYER5,
    WAIT_ACK
};

struct Sender
{
    enum SenderState state;
    int seq; // número do último pacote enviado por A
    float estimated_rtt;
    struct pkt last_packet;
} A ;

struct Receiver
{
    int seq; // número do último ACK recebibo por B
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

/* chamado da camada 5, passou os dados para serem enviados para o outro lado */
void A_output(struct msg message)
{
    if (A.state != WAIT_LAYER5)
    {
        printf("  A_output: not yet acked. drop the message: %s\n", message.data);
        return;
    }
    printf("  A_output: enviar pacote: %s\n", message.data);
    struct pkt packet; // cria estrutura pacote
    packet.seqnum = A.seq; // atribui o número da sequencia
    memmove(packet.payload, message.data, 20); // encapsula a o pacote
    packet.checksum = get_checksum(&packet); // cria o número verificador do pacote
    A.last_packet = packet; // guarda o último pacote enviado, caso se perca ou se corrumpa
    A.state = WAIT_ACK; // define o estado do pacote para esperando ack
    tolayer3(0, packet); // A envia o pacote para a camada de transporte (0-A 1-B)
    starttimer(0, A.estimated_rtt); // inicia o temporizador no lado A (0-A 1-B)
}

/* precisa ser preenchido apenas para crédito extra */
void B_output(struct msg message)
{
    printf("  B_output: uni-directional. ignore.\n");
}

/* chamado da camada 3, quando um pacote chega para a camada 4 */
void A_input(struct pkt packet)
{
    if (A.state != WAIT_ACK) // Se A não está esperando ACK de B, devido timeout
    {
        printf("  A_input: Timeout. descartar.\n");
        return;
    }
    if (packet.checksum != get_checksum(&packet)) // se ACK chegou chegou corrumpido
    {
        printf("  A_input: ACK corrumpido. descartar.\n");
        return;
    }

    if (packet.acknum != A.seq)
    {
        printf("  A_input: Numero de ACK nao esperado. descartar.\n");
        return;
    }
    printf("  A_input: ACK recebido.\n");
    stoptimer(0);
    A.seq = 1 - A.seq; //bit alternate
    A.state = WAIT_LAYER5; // não está esperando ACK
}

/* chamado quando o temporizador de A se apaga */
void A_timerinterrupt(void) // timeout
{
    if (A.state != WAIT_ACK)
    {
        printf("  A_timerinterrupt: nao esta esperando ACK. ignorar evento.\n");
        return;
    }
    printf("  A_timerinterrupt: reenviar o último pacote: %s.\n", A.last_packet.payload);
    tolayer3(0, A.last_packet);
    starttimer(0, A.estimated_rtt);
}


/* a seguinte rotina será chamada uma vez (somente) antes de qualquer outro */
/* entidade rotinas são chamadas. Você pode usá-lo para fazer qualquer inicialização */
void A_init(void)
{
    A.state = WAIT_LAYER5;
    A.seq = 0;
    A.estimated_rtt = 15;
}

/* Note que com a transferência simplex de um para B, não há B_output () */

void send_ack(int AorB, int ack)
{
    struct pkt packet;
    packet.acknum = ack;
    packet.checksum = get_checksum(&packet);
    tolayer3(AorB, packet); // A ou B envia ACK para a camada de rede
}

/* chamado da camada 3, quando um pacote chega para a camada 4 em B */
void B_input(struct pkt packet)
{
    if (packet.checksum != get_checksum(&packet))
    {
        printf("  B_input: pacote corrumpido. enviar NAK.\n");
        send_ack(1, 1 - B.seq); // envia o último ACK recebido corretamente (1-1 = 0 / 1-0 = 1 (bit alternate))
        return;
    }
    if (packet.seqnum != B.seq)
    {
        printf("  B_input: numero de sequencia nao esperado. enviar NAK.\n");
        send_ack(1, 1 - B.seq); // envia o último ACK recebido corretamente (1-1 = 0 / 1-0 = 1 (bit alternate))
        return;
    }
    printf("  B_input: recuperar a mensagem (recv): %s\n", packet.payload); // recupera a mensagem enviada por A
    printf("  B_input: enviar ACK (send).\n"); // envia o ACK
    send_ack(1, B.seq);
    tolayer5(1, packet.payload); // envia o pacote para a camada de aplicação
    B.seq = 1 - B.seq; // alterna o bit
}

/* 
chamado quando o temporizador de B se apaga */
void B_timerinterrupt(void)
{
    printf("  B_timerinterrupt: B doesn't have a timer. ignore.\n");
}


/* o seguinte rotytine será chamado uma vez (somente) antes de qualquer outro */
/* as rotinas da entidade B são chamadas. Você pode usá-lo para fazer qualquer inicialização */
void B_init(void)
{
    B.seq = 0;
}

/************************************************* ****************
***************** O CÓDIGO DE EMULAÇÃO DE REDE SE INICIA ABAIXO ***********
O código abaixo emula a camada 3 e abaixo do ambiente de rede:
    - emula a transmissão e a entrega (possivelmente com corrupção em nível de bit
        e perda de pacotes) de pacotes através da camada de interface 3/4
    - lida com o início / parada de um temporizador e gera temporizador
        interrupções (resultando na chamada de manipulador de temporizador de alunos).
    - gera mensagem para ser enviada (passada de 5 para 4)
NÃO HÁ RAZÃO QUE QUALQUER ALUNO DEVERIA LER OU ENTENDER
O CÓDIGO ABAIXO. VOCÊ NÃO TOCAR OU TOCAR (em seu código) QUALQUER
DAS ESTRUTURAS DE DADOS ABAIXO. Se você está interessado em como eu projetei
o emulador, você está convidado a olhar para o código - mas, novamente, você deve ter
para, e você defeinitely não deveria ter que modificar
************************************************** ****************/

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
