#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <locale.h>

#define PILHA_TAM 8194 //Tamanho da pilha
#define TRANS_TAM 3 //Tamanho máximo de nome das transições
#define PROD_NUM 10 //Número de transições E símbolos (vetores 'S' e 'B')
#define AMOSTRA_NUM 4096 // Número de amostras
#define ARV_TAM 256 //Tamanho da árvore de transições
#define DIST 1 //Distância entre os filhos inseridos na árvore-vetor

//Protótipos de funções
char inicializaProd(void); //Inicializa os vetores de produções 'S' e 'B' e adiciona no topo da pilha o primeiro caractere não terminal
void mostraS(void); //Mostra a vetor de símbolos 'S'
void mostraB(void); //Mostra a vetor de conectivos 'B'
int buscaTransicaoS(char c); //Retorna a posição de um "transição" no vetor 'S' que deve ser empilhada
int buscaTransicaoB(char c); //Retorna a posição de um "transição" no vetor 'B' que deve ser empilhada
int pegaEntrada(void); //Pega a cadeia de entrada
void mostraPilha(void); //Exibe a pilha
void mostraEntrada(void); //Exibe a cadeia de entrada
void mostraProducoes(void);
void notificarErroGramatica(void); //Notifica erro de gramática.
int pop(void); //Remove o elemento do topo da pilha
int push(char e); //Insere um elemento no topo da pilha
int empty(void); //Retorna 1 se a pilha estiver vazia e 0 caso contrário
char read(void); //Consome o elemento atual da entrada e faz pop na pilha
void init(void); //Inicializa variáveis e vetores
void iniciaAutomato(void); //Inicia a verificação de sintaxe
//Funções de árvore
void insereArvore(int pai,char N, int transicao); //Insere na árvore um filho d a transição 'pos' do não-terminal 'N'
void mostraArvore(void); //Exibe a àrvore-vetor
void resetaArvore(void); //Reseta a árvore
void criaArvore(void); //Insere as transições na árvore de produções

//Variáveis globais
char S[PROD_NUM][PROD_NUM], B[PROD_NUM][PROD_NUM], naoTermS, naoTermB;
char producoes[PROD_NUM][PROD_NUM];
char arvore[ARV_TAM]; //Árvore de análise sintática
int flag_setado_NaoTermB = 0; //Flag que determina se o símbolo não terminal B já foi setado
char pilha[PILHA_TAM]; //Pilha
char entrada[PILHA_TAM]; //Cadeia de entrada
int topo = -1; //Guarda o topo da pilha
int carret = -1; //Guarda a posição do cursor que varrerá a antrada
int prod_carret = -1;
FILE *amostra, *resultados; //Arquivos de amostra e do resultado da análise
FILE *prod; //Arquivo de produções
int flag_escrevendo = -1; //1 se o resultado estiver sendo escrito num arquivo externo
int carret_amostra = -1; //Carret que percorrerá o arquivo de amostras
int fim_tree = 0, k_tree = 0; //Garda o fim da árvore e a próxima posição a partir de onde se pode escrever no vetor

int main(int argc, const char* argv[])
{
    setlocale(LC_ALL, "Portuguese"); //Permite acentuação
    
    if((prod = fopen(argv[1],"r")) && (argc == 2)){ //Foi carregado somente o arquivo de produções
    	flag_escrevendo = 0; //Desabilita escrita
    	
		printf("\n-> Arquivo carregado: %s\n",argv[1]);
		while(pegaEntrada())
			iniciaAutomato();
	} else {
		if((argc == 3) && (prod = fopen(argv[1],"r"))){ //Foi carregado um arquivo de produções e um de amostra
			printf("\n-> Arquivo de produções carregado: %s\n",argv[1]);
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
				printf("\nAnálise realizada com sucesso!\n\n");
				exit(1);
			}
			else{
				printf("\nO arquivo de amostras não pôde ser encontrado!\nCertifique-se de que o arquivo se encontra no mesmo diretório do analisador\n");
			}
		}
	}

	fclose(prod);
    return 0;
}

char inicializaProd(){
	char ch,auxString[PILHA_TAM];
	int i,j=0;
	rewind(prod); //Volta o ponteiro para o início do arquivo (colocado aqui porque essa função é chamada toda vez que uma cadeia de entrada é analisada)
	naoTermS = fgetc(prod); //Pega o primeiro caractere do arquivo, supostamente o não-terminal inicial
	rewind(prod); //Volta o ponteiro para o início do arquivo
	
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
		fprintf(resultados, "%s - A FÓRMULA DIGITADA NÃO FAZ PARTE DA GRAMÁTICA!\n", entrada);
	}
	else{
		printf("\nA FÓRMULA DIGITADA NÃO FAZ PARTE DA GRAMÁTICA!\n\n");
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
    pilha[0] = inicializaProd(); //Inicia a pilha com o primeiro caractere não terminal do arquivo
    producoes[0][0] = 'S';
    arvore[0] = 'S';
    k_tree = 1;
    topo = 0; // Ajusta o topo da pilha
    carret = 0; //Ajusta o cursor para o início da entrada
    prod_carret = 1;
}

void iniciaAutomato(){
    int i;
	init();
    
    while(entrada[carret]!='\0'){
    	
    	char trans[PROD_NUM]; //Guarda a cadeia que será empilhada
    	
    	if((entrada[carret] == pilha[topo]) && (entrada[carret] != naoTermS) && (entrada[carret] != naoTermB)){ //Se o caractere atual está no topo e não é nenhum dos sílbolos não terminais...
    		read();
		} else{
			int transicao; //Guarda a posição da transição que será usada
			//Deve-se verificar se a transição é pra ser buscada no vetor 'S' ou no vetor 'B'
			if(pilha[topo]==naoTermB){ //Verifica se o que está no topo da pilha é o segundo não-terminal (B)
				transicao = buscaTransicaoB(entrada[carret]); //Busca uma produção com o segundo não-terminal
				if(transicao == -1){
					notificarErroGramatica();
					return;
				} else {
					strcpy(trans,B[transicao]);
					strcpy(producoes[prod_carret++],trans);
				}
				
			} else { //senão for o segundo não-terminal, verifica se é o primeiro não-terminal (S) que está no topo
				if(pilha[topo]==naoTermS){
					transicao = buscaTransicaoS(entrada[carret]); //Busca uma produção com o primeiro não-terminal
					if(transicao == -1){
						notificarErroGramatica();
						return;
					} else {
						strcpy(trans,S[transicao]);
						strcpy(producoes[prod_carret++],trans);
					}
				}
			}
			
			//Troca o topo da pilha pela transição selecionada
			if((pilha[topo] == naoTermS || pilha[topo] == naoTermB)){ //No topo deve ter algum não terminal para que se aplique a produção	
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
		    	fprintf(resultados,"%s - FÓRMULA BEM FORMADA ACEITA!\n",entrada);
			} else {
				printf("\nFÓRMULA BEM FORMADA ACEITA!\n\n");
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
			if(k_tree==0){ //A árvore tá vazia
			arvore[k_tree++]='S';
			
			/*	for(k_tree=0;k_tree<strlen(S[transicao]);k_tree++){
					arvore[k_tree] = S[transicao][k_tree];//-1 senão pula o primeiro caractere da produção
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
	if(fim_tree < k_tree*pai+DIST+i) //Reajusta o fim da árvore no vetor
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
		
		for(;!isupper(arvore[pai]);pai++); //Procura a posição do próximo pai
		
		printf("Inserindo produção \"%s\" filho de \'%c\'[%d]\n",producoes[i],arvore[pai],pai);
		
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
