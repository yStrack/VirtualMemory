#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/*
Grupo:
1- Yuri Marques Strack
*/

struct pagina{
  int indice; //endereco da pagina
  int r; // flag de pagina referenciada
  int m; // flag de pagina modificada
  int tempoAcesso; //instante do ultimo acesso da pagina em memoria
  int presenca; // 0 se n estiver em memoria,1 se estiver
};
typedef struct pagina Pagina;

int tamPagina,espaco; // tamanho na pagina e tamanho da memoria fisica
char nome[40],tipo[4]; // nome do arquivo p/ leitura e tipo de substituicao (lru,nru)
double tamanho;
int tamanhoF;
Pagina** tabPagina; //tabela de paginas
Pagina** memoria; // memoria fisica

int tempo = 0;
int pageFault = 0;
int pageHit = 0;
int pageMiss = 0;
int paginaSuja = 0; //printar quando ha troca
int numPag = 0; //numero de paginas em memoria
int limite; // total de paginas possivel em memoria (tamMemoria/tamPag)
int totalPag = 0; //numero de paginas
int reads=0,writes=0; //numero de reads e writes;
int debug = 0;

int calculaS(int tamP){ //calcula tamanho do >>
  int s = (tamP/8) + 12;
  return s;
}

int buscaPagina(int endereco){ // busca na tab de paginas p/ verificar se ela ja existe
  int i;
  if (tabPagina[endereco] == NULL){
    return 0;
  }
  else{
    return 1;
  }
}

int buscaMemoria(Pagina* p){ //busca em memoria, return -1 se n estiver, indice se estiver
  int i;
  if(tabPagina[p->indice]->presenca == 0){
    return -1;
  }
  else{
    for(i=0;i<limite;i++){
      if (memoria[i]->indice == p->indice){
        return i;
      }
    }
  }
}

void zeraR(){
  int i=0;
  for (;i<limite;i++){
    if (memoria[i] == NULL){
      return;
    }
    memoria[i]->r=0;
  }
}

void LRU(Pagina* p){ // metodo LRU
  int i=0;
  int aux=0;
  int menorT = memoria[0]->tempoAcesso;
  for (;i<limite;i++){
    if (memoria[i]->tempoAcesso < menorT){
      menorT = memoria[i]->tempoAcesso;
      aux = i;
    }
  }
  tabPagina[p->indice]->presenca =1;
  tabPagina[p->indice]->tempoAcesso =tempo;
  tabPagina[memoria[aux]->indice]->presenca = 0;
  tabPagina[memoria[aux]->indice]->r=0;
  if (debug){printf("Pagina %x removida do endereco de memoria %d!\n",memoria[aux]->indice,aux);}
  if (memoria[aux]->m == 1){
    if (debug){printf("Pagina suja copiada para disco!\n");}
    paginaSuja++;
  }
  tabPagina[memoria[aux]->indice]->m=0;
  memoria[aux] = tabPagina[p->indice];
  if (debug){printf("Pagina %x inserida no endereco de memoria %d!\n",p->indice,aux);}
}

void NRU(Pagina* p){ //metodo NRU
  int i=0,j=0,a=0,b=0;
  int aux=0,flag=0;
  for (;i<limite;i++){
    if (memoria[i]->r == 0 && memoria[i]->m == 0){
      aux = i;
      flag=1;
      break;
    }
  }
  if(flag==0){
    for (i=0;i<limite;i++){
      if (memoria[i]->r == 0 && memoria[i]->m == 1){
        aux = i;
        flag=1;
        break;
      }
    }
  }
  if (flag==0){
    for (i=0;i<limite;i++){
      if (memoria[i]->r == 1 && memoria[i]->m == 0){
        aux = i;
        flag=1;
        break;
      }
    }
  }
  if (flag==0){
    for (i=0;i<limite;i++){
      if (memoria[i]->r == 1 && memoria[i]->m == 1){
        aux = i;
        flag=1;
        break;
      }
    }
  }
  tabPagina[p->indice]->presenca =1;
  tabPagina[p->indice]->tempoAcesso =tempo;
  tabPagina[memoria[aux]->indice]->presenca = 0;
  tabPagina[memoria[aux]->indice]->r=0;
  if (debug){printf("Pagina %x removida do endereco de memoria %d!\n",memoria[aux]->indice,aux);}
  if (memoria[aux]->m == 1){
    if (debug){printf("Pagina suja copiada para disco!\n");}
    paginaSuja++;
  }
  tabPagina[memoria[aux]->indice]->m=0;
  memoria[aux] = tabPagina[p->indice];
  if (debug){printf("Pagina %x inserida no endereco de memoria %d!\n",p->indice,aux);}
}

void trocaPagina(Pagina* p){ //swap de paginas
  if (strcmp(tipo,"LRU") == 0 || strcmp(tipo,"lru") == 0){
    if (debug){printf("Algoritmo de substituicao: LRU!\n");}
    LRU(p);
  }
  else{
    if (debug){printf("Algoritmo de substituicao: NRU!\n");}
    NRU(p);
  }
}

void insereTabPag(Pagina* p){
  tabPagina[p->indice] = p;
  totalPag++;
}

void escrevePag(Pagina* p){
  //printf("entrei no escrevePag\n" );
  if (numPag < limite){
    if (debug){printf("A memoria nao esta cheia, possivel inserir a pagina!\n");}
    p->presenca = 1;
    memoria[numPag] = p;
    if (debug){printf("Pagina %x inserida no endereco %d da memoria\n",p->indice,numPag);}
    numPag++;
  }
  else{
    if (debug){printf("Memoria cheia, sem espaco para colocar a pagina!\n");}
    if (debug){printf("Precisa realizar troca de pagina!\n");}
    trocaPagina(p);
  }
  return;
}

void leArq(){
  tempo = 0; // equivalente ao numero de operacoes
  unsigned int addr; //endereco de memoria
  char rw; //read or write
  FILE* f;
  f = fopen(nome,"r");
  while(EOF != fscanf(f,"%x %c", &addr, &rw)){
    Pagina* p;
    int endereco,e,b;
    char tipo;
    if (debug){printf("Endereco lido:%x\n",addr);}
    endereco = addr >> calculaS(tamPagina);
    if (debug){printf("Pagina: %x\n",endereco);}
    e = buscaPagina(endereco);
    if (e == 0){ // se ela nao existir ainda, cria ela
      p =(Pagina*) malloc(sizeof(Pagina));
      p->indice = endereco;
      p->m = 0;
      p->r = 0;
      p->tempoAcesso = tempo;
      p->presenca = 0;
      insereTabPag(p);
    }
    if (debug){printf("Verificando se a pagina esta em memoria\n");}
    b= buscaMemoria(tabPagina[endereco]); //verifica se esta em memoria
    if(rw == 'W' || rw == 'w'){ // se for para escrita atualiza flag m
      tabPagina[endereco]->m = 1;
      writes++;
    }
    else if(rw == 'R' || rw == 'r'){ // se for para leitura atualiza flag r
      tabPagina[endereco]->r = 1;
      reads++;
    }
    if (b == -1){ // se nao estiver em memoria
      pageFault++;
      if (debug){printf("A pagina nao esta na memoria\nPage Fault\n");}
      escrevePag(tabPagina[endereco]); //coloca na memoria
    }
    else{ // se estiver em memoria
      pageHit++;
      if (debug){printf("A pagina esta na memoria\nPage Hit\n");}
      memoria[b]->tempoAcesso = tempo; //atualiza o tempo de acesso
    }
    tempo++;
    if(tempo%limite == 0){
      if(debug) {printf("Zerando as flags R das paginas em memoria\n");getchar();}
      zeraR();
    }
    if(debug) {printf("Aperte enter para continuar\n");getchar();}
  }
  fclose(f);
}

int main(int argc,char* argv[]){
  int i = 0;
  debug=0;
  tamPagina = atoi(argv[3]); //tam do quadro
  espaco = atoi(argv[4]) * 1024; //total de memória física hipoteticamente disponível
  strcpy(nome,argv[2]);
  strcpy(tipo,argv[1]);
  if (argv[5]){
    debug = 1;
  }
  limite = espaco/tamPagina;
  tamanho = pow(2,32-calculaS(tamPagina));
  tamanhoF = floor(tamanho);
  tabPagina =(Pagina**) malloc(tamanhoF*sizeof(Pagina*));
  memoria = (Pagina**) malloc(limite*sizeof(Pagina*));
  for (i=0;i<limite;i++){
    memoria[i] = NULL;
  }
  for (i=0;i<tamanhoF;i++){
    tabPagina[i] = NULL;
  }
  leArq();
  for (;i<limite;i++){
    free(memoria[i]);
  }
  for (;i<tamanhoF;i++){
    free(tabPagina[i]);
  }
  free(tabPagina);
  free(memoria);
  printf("\nExecutando o simulador...\n");
	printf("Arquivo de entrada %s\n", nome);
	printf("Tamanho da memoria fisica: %dMB\n", espaco/1024);
  printf("Tamanho das paginas: %dKB\n", tamPagina);
  printf("Alg de substituicao: %s\n", tipo);
	printf("Numero de Faltas de pagina: %d\n", pageFault);
	printf("Numero de paginas escritas: %d\n", paginaSuja);
  if (debug){
    printf("Numero de paginas: %d\n", totalPag);
    printf("Numero de leituras: %d\n", reads);
    printf("Numero de escritas: %d\n", writes);
    printf("Numero de operacoes%d\n", tempo);
  }
  return 0;
}
