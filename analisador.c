#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <locale.h>

#define PILHA_TAM 8194 //Tamanho da pilha
#define TRANS_TAM 3 //Tamanho m�ximo de nome das transi��es
#define PROD_NUM 10 //N�mero de transi��es E s�mbolos (vetores 'S' e 'B')
#define AMOSTRA_NUM 4096 // N�mero de amostras
#define ARV_TAM 256 //Tamanho da �rvore de transi��es
#define DIST 1 //Dist�ncia entre os filhos inseridos na �rvore-vetor

//Prot�tipos de fun��es
char inicializaProd(void); //Inicializa os vetores de produ��es 'S' e 'B' e adiciona no topo da pilha o primeiro caractere n�o terminal
void mostraS(void); //Mostra a vetor de s�mbolos 'S'
void mostraB(void); //Mostra a vetor de conectivos 'B'
int buscaTransicaoS(char c); //Retorna a posi��o de um "transi��o" no vetor 'S' que deve ser empilhada
int buscaTransicaoB(char c); //Retorna a posi��o de um "transi��o" no vetor 'B' que deve ser empilhada
int pegaEntrada(void); //Pega a cadeia de entrada
void mostraPilha(void); //Exibe a pilha
void mostraEntrada(void); //Exibe a cadeia de entrada
void mostraProducoes(void);
void notificarErroGramatica(void); //Notifica erro de gram�tica.
int pop(void); //Remove o elemento do topo da pilha
int push(char e); //Insere um elemento no topo da pilha
int empty(void); //Retorna 1 se a pilha estiver vazia e 0 caso contr�rio
char read(void); //Consome o elemento atual da entrada e faz pop na pilha
void init(void); //Inicializa vari�veis e vetores
void iniciaAutomato(void); //Inicia a verifica��o de sintaxe
//Fun��es de �rvore
void insereArvore(int pai,char N, int transicao); //Insere na �rvore um filho d a transi��o 'pos' do n�o-terminal 'N'
void mostraArvore(void); //Exibe a �rvore-vetor
void resetaArvore(void); //Reseta a �rvore
void criaArvore(void); //Insere as transi��es na �rvore de produ��es

//Vari�veis globais
char S[PROD_NUM][PROD_NUM], B[PROD_NUM][PROD_NUM], naoTermS, naoTermB;
char producoes[PROD_NUM][PROD_NUM];
char arvore[ARV_TAM]; //�rvore de an�lise sint�tica
int flag_setado_NaoTermB = 0; //Flag que determina se o s�mbolo n�o terminal B j� foi setado
char pilha[PILHA_TAM]; //Pilha
char entrada[PILHA_TAM]; //Cadeia de entrada
int topo = -1; //Guarda o topo da pilha
int carret = -1; //Guarda a posi��o do cursor que varrer� a antrada
int prod_carret = -1;
FILE *amostra, *resultados; //Arquivos de amostra e do resultado da an�lise
FILE *prod; //Arquivo de produ��es
int flag_escrevendo = -1; //1 se o resultado estiver sendo escrito num arquivo externo
int carret_amostra = -1; //Carret que percorrer� o arquivo de amostras
int fim_tree = 0, k_tree = 0; //Garda o fim da �rvore e a pr�xima posi��o a partir de onde se pode escrever no vetor

int main(int argc, const char* argv[])
{
    setlocale(LC_ALL, "Portuguese"); //Permite acentua��o
    
    if((prod = fopen(argv[1],"r")) && (argc == 2)){ //Foi carregado somente o arquivo de produ��es
    	flag_escrevendo = 0; //Desabilita escrita
    	
		printf("\n-> Arquivo carregado: %s\n",argv[1]);
		while(pegaEntrada())
			iniciaAutomato();
	} else {
		if((argc == 3) && (prod = fopen(argv[1],"r"))){ //Foi carregado um arquivo de produ��es e um de amostra
			printf("\n-> Arquivo de produ��es carregado: %s\n",argv[1]);
			flag_escrevendo = 1; //Habilita escrita
			if((amostra = fopen(argv[2],"r"))){
				printf("-> Arquivo de amostras carregado: %s\n",argv[2]);
				char nome_arquivo_resultado[] = "resultado-";
				strcat(nome_arquivo_resultado,argv[2]);
				printf("-> Arquivo de resultados: %s\n",nome_arquivo_resultado);
				resultados = fopen(nome_arquivo_resultado,"w"); //Cria arquivo para escrever resultados
				for(carret_amostra=0;carret_amostra<AMOSTRA_NUM;carret_amostra++){
        			fscanf(amostra,"%s ",entrada);
        			iniciaAutomato();
    			}
   				fclose(amostra);
				fclose(resultados);
				printf("\nAn�lise realizada com sucesso!\n\n");
				exit(1);
			}
			else{
				printf("\nO arquivo de amostras n�o p�de ser encontrado!\nCertifique-se de que o arquivo se encontra no mesmo diret�rio do analisador\n");
			}
		}
	}

	fclose(prod);
    return 0;
}

char inicializaProd(){
	char ch,auxString[PILHA_TAM];
	int i,j=0;
	rewind(prod); //Volta o ponteiro para o in�cio do arquivo (colocado aqui porque essa fun��o � chamada toda vez que uma cadeia de entrada � analisada)
	naoTermS = fgetc(prod); //Pega o primeiro caractere do arquivo, supostamente o n�o-terminal inicial
	rewind(prod); //Volta o ponteiro para o in�cio do arquivo
	
	for(i=0;(ch=fgetc(prod)) != EOF;i++){
		if(ch == naoTermS){
			fscanf(prod,"%s ",auxString);
			strcpy(S[i],auxString);
		} else {
			if(!flag_setado_NaoTermB){
				naoTermB = ch;
				flag_setado_NaoTermB = 1;
			}
			
			fscanf(prod,"%s ",auxString);
			strcpy(B[j++],auxString);
		}
	}
	
	return naoTermS;
}

void mostraS(){
	int i;
	for(i=0;S[i][0]!='\0';i++){
		printf("P%d -> %s\n",i,S[i]);
	}
}

void mostraB(){
	int i;
	for(i=0;B[i][0]!='\0';i++){
		printf("B%d -> %s\n",i,B[i]);
	}
}

int buscaTransicaoS(char c){
	int i;
	for(i=0; i<PROD_NUM; i++){
		if(c == S[i][0]){
			return i;
		}
	}
	return -1;
}

int buscaTransicaoB(char c){
	int i;
	for(i=0; i<PROD_NUM; i++){
		if(c == B[i][0]){
			return i;
		}
	}
	return -1;
}

int pegaEntrada(){
    printf("Insira uma string: [CTRL + C para sair]\n");
    return scanf("%s",&entrada);
}

void mostraPilha(){
    int i;
    printf("Pilha -> ");
    for(i=0;i<=topo;i++){
        printf("|");
        printf("%c",pilha[i]);
    }
    printf(" <- topo\n\n");
}

void mostraEntrada(){
    printf("Entrada: %s",entrada);
}
void mostraProducoes(){
	int i;
	for(i=0;producoes[i][0]!='\0';i++){
		puts(producoes[i]);
	}
}

void notificarErroGramatica(){
	if(flag_escrevendo){
		fprintf(resultados, "%s - A F�RMULA DIGITADA N�O FAZ PARTE DA GRAM�TICA!\n", entrada);
	}
	else{
		printf("\nA F�RMULA DIGITADA N�O FAZ PARTE DA GRAM�TICA!\n\n");
	}
}

int pop(){
    if(topo>=0){
        topo--;
        return 1;
    }
    return 0;
}

int push(char e){
    if(topo<PILHA_TAM+1){
        pilha[++topo]=e;
        return 1;
    }
    return 0;
}

int empty(){
	return (topo<0?1:0);
}

char read(){
    char c = pilha[topo];
    if(topo>=0){
        pop();
        carret++;
        return c;
    }
    return ' ';
}

void init(){
    pilha[0] = inicializaProd(); //Inicia a pilha com o primeiro caractere n�o terminal do arquivo
    producoes[0][0] = 'S';
    arvore[0] = 'S';
    k_tree = 1;
    topo = 0; // Ajusta o topo da pilha
    carret = 0; //Ajusta o cursor para o in�cio da entrada
    prod_carret = 1;
}

void iniciaAutomato(){
    int i;
	init();
    
    while(entrada[carret]!='\0'){
    	
    	char trans[PROD_NUM]; //Guarda a cadeia que ser� empilhada
    	
    	if((entrada[carret] == pilha[topo]) && (entrada[carret] != naoTermS) && (entrada[carret] != naoTermB)){ //Se o caractere atual est� no topo e n�o � nenhum dos s�lbolos n�o terminais...
    		read();
		} else{
			int transicao; //Guarda a posi��o da transi��o que ser� usada
			//Deve-se verificar se a transi��o � pra ser buscada no vetor 'S' ou no vetor 'B'
			if(pilha[topo]==naoTermB){ //Verifica se o que est� no topo da pilha � o segundo n�o-terminal (B)
				transicao = buscaTransicaoB(entrada[carret]); //Busca uma produ��o com o segundo n�o-terminal
				if(transicao == -1){
					notificarErroGramatica();
					return;
				} else {
					strcpy(trans,B[transicao]);
					strcpy(producoes[prod_carret++],trans);
				}
				
			} else { //sen�o for o segundo n�o-terminal, verifica se � o primeiro n�o-terminal (S) que est� no topo
				if(pilha[topo]==naoTermS){
					transicao = buscaTransicaoS(entrada[carret]); //Busca uma produ��o com o primeiro n�o-terminal
					if(transicao == -1){
						notificarErroGramatica();
						return;
					} else {
						strcpy(trans,S[transicao]);
						strcpy(producoes[prod_carret++],trans);
					}
				}
			}
			
			//Troca o topo da pilha pela transi��o selecionada
			if((pilha[topo] == naoTermS || pilha[topo] == naoTermB)){ //No topo deve ter algum n�o terminal para que se aplique a produ��o	
				pop();
				for(i=strlen(trans)-1;i>=0;i--){
					push(trans[i]);
				}
			} else {
				notificarErroGramatica();
				return;
			}
		}
    }
    
    if(empty()){ //Se a pilha estiver vazia...
		if(carret == strlen(entrada)){ //Se toda a cadeia foi lida...
			if(flag_escrevendo){
		    	fprintf(resultados,"%s - F�RMULA BEM FORMADA ACEITA!\n",entrada);
			} else {
				printf("\nF�RMULA BEM FORMADA ACEITA!\n\n");
				mostraProducoes();
				criaArvore();
				mostraArvore();
			}
		} else {
			notificarErroGramatica();
		}
	} else {
		notificarErroGramatica();
	}
	
	//mostraProducoes();
	
	//producoesToArvore();
}

void insereArvore(int pai, char N, int transicao){
	int i;
	
	switch(N){
		case 'S':
			if(k_tree==0){ //A �rvore t� vazia
			arvore[k_tree++]='S';
			
			/*	for(k_tree=0;k_tree<strlen(S[transicao]);k_tree++){
					arvore[k_tree] = S[transicao][k_tree];//-1 sen�o pula o primeiro caractere da produ��o
				}*/
				fim_tree = k_tree;
			} else {
				for(i=0;i<strlen(S[transicao]);i++){
					printf("Inserindo na posicao %d da arvore: %c\n",k_tree*pai+DIST+i,S[transicao][i]);
					arvore[k_tree*pai+DIST+i] = S[transicao][i];
				}
			}
			break;
		case 'B':
			if(k_tree==1){
				for(k_tree=1;k_tree<strlen(B[transicao]);k_tree++){
					arvore[k_tree] = B[transicao][k_tree-1];
				}
				fim_tree = k_tree;
			} else {
				for(i=0;i<strlen(B[transicao]);i++){
					printf("Inserindo na posicao %d da arvore: %c\n",k_tree*pai+DIST+i,B[transicao][i]);
					arvore[k_tree*pai+DIST+i] = B[transicao][i];
				}
			}
			break;
	}
	if(fim_tree < k_tree*pai+DIST+i) //Reajusta o fim da �rvore no vetor
		fim_tree = k_tree*pai+DIST+i;
}

void mostraArvore(){
	int i;
	printf("Tree -> ");
	for(i=0;i<=fim_tree;i++){
		printf(" %c |",arvore[i]);
	}
	printf("\nFim: %d\n",fim_tree);
}

void resetaArvore(){
	fim_tree = 0;
	k_tree = 0;
}

void criaArvore(){
	int i, j, arvore_carret=0, pai=0;
	
	arvore[0]='S';
	arvore_carret++;
	k_tree=1;
	fim_tree++;
	
	for(i=1;producoes[i][0]!='\0';i++){
		
		for(;!isupper(arvore[pai]);pai++); //Procura a posi��o do pr�ximo pai
		
		printf("Inserindo produ��o \"%s\" filho de \'%c\'[%d]\n",producoes[i],arvore[pai],pai);
		
		for(j=0;producoes[i][j]!='\0';j++){
			printf("Inserindo filho em [%d] do pai [%d]\n",k_tree*pai+DIST+j,pai);
			arvore[k_tree*pai+DIST+j] = producoes[i][j];
		}
		fim_tree=k_tree*pai+j;
		if(j>k_tree)
			k_tree=j;
		pai++;
	}
}
