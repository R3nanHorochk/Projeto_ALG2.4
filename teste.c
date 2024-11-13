#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <stdint.h>

#define MAXIMO_PALAVRAS 24
#define MAXIMO_COMBINACOES 7962624
#define TAMANHO_PALAVRAS 58

int Base64Encode(const unsigned char* buffer, size_t length, char** b64text) { //Encodes a binary safe base 64 string
	BIO *bio, *b64;
	BUF_MEM *bufferPtr;

	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new(BIO_s_mem());
	bio = BIO_push(b64, bio);

	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Ignore newlines - write everything in one line
	BIO_write(bio, buffer, length);
	BIO_flush(bio);
	BIO_get_mem_ptr(bio, &bufferPtr);
	BIO_set_close(bio, BIO_NOCLOSE);
	BIO_free_all(bio);

	*b64text=(*bufferPtr).data;

	return (0); //success
}

char* ler_proxima_palavra(FILE *pont_dados) { //pega uma palavra do arquivo 
    static char palavra[100];
    if (fscanf(pont_dados, "%s", palavra) == 1) {
        return palavra;
    } else {
        return NULL;
    }
}

char* ler_proxima_palavra2(FILE *pont_dados) { //pega uma linha do arquivo
    static char linha[100];
    if (fgets(linha, sizeof(linha), pont_dados) != NULL) {
        strtok(linha, "\n");  //remove o '\n' da linha para que a hashe gerada seja a correta
        return linha;
    } else {
        return NULL;
    }
}

void guardar_possibilidades(FILE *possibilidades, char name1[], char name2[], char name3[], char name4[], char name5[]) {
    fprintf(possibilidades, "%s %s %s %s %s\n", name1, name2, name3, name4, name5);
}

void guardar_hash(FILE *hashes, char hash_hex[]) {
    if (hashes != NULL) {
        fprintf(hashes, "%s\n", hash_hex);
    }
}


void gerar_combinacoes(char vetor_palavras[][TAMANHO_PALAVRAS], int n) { //gerando todas as possibilidades de senha
    int cont = 0;
    bool parar = false;

    FILE *possibilidades = fopen("todas_possibilidades.txt", "a");
    if (possibilidades == NULL) {
        printf("Erro ao abrir o arquivo para gravação de possibilidades!\n");
        return;
    }

    for (int b = 0; b < n && !parar; b++) { //enquanto parar == false os loops rodam
        for (int c = 0; c < n && !parar; c++) {
            for (int d = 0; d < n && !parar; d++) {
                for (int e = 0; e < n && !parar; e++) {
                    for (int f = 0; f < n && !parar; f++) {
                        if (cont < MAXIMO_COMBINACOES) { //garantindo que as combinacoes nao ultrapasse o tamanho de todas as possibilidades
                            cont++;
                            guardar_possibilidades(possibilidades, vetor_palavras[b], vetor_palavras[c], vetor_palavras[d], vetor_palavras[e], vetor_palavras[f]);
                        } else {
                            parar = true;
                            break;
                        }
                    }
                }
            }
        }
    }
    fclose(possibilidades);
}

int retornar_linha_hash (FILE *hashes, FILE *senhas_quebradas) {

}

void gerar_hashes(unsigned char *hash, char *hash_hex, FILE *possibilidades_senhas, SHA512_CTX *ctx, FILE *hashes) { //gerando as hashes a partir do arquivo todas_possibilidades.txt
    char *linha;
    while ((linha = ler_proxima_palavra2(possibilidades_senhas)) != NULL) {
        SHA512_Init(ctx);
        SHA512_Update(ctx, linha, strlen(linha));
        SHA512_Final(hash, ctx);
        char *base64encoded;
        Base64Encode(hash, 64, &base64encoded);

        
        
        guardar_hash(hashes, base64encoded);
    }
}

void remover_nome_e_pontos(FILE *arquivo_original, FILE *arquivo_limpo) {
    char linha[200];

    while (fgets(linha, sizeof(linha), arquivo_original) != NULL) {
        char *hash = strchr(linha, ':');
        if (hash != NULL) {
            fprintf(arquivo_limpo, "%s", hash + 1);
        }
    }
}

/*int verificar_palavra(FILE *palavra, char palavra[]) { // verifica se a palavra digitada esta no arquivo sem acentos.txt
  char acumular_conteudo_linha[50];
  int n_linha;
  rewind(palavra); // voltar para o inicio do arquivo

  while (fgets(acumular_conteudo_linha, sizeof(acumular_conteudo_linha),palavra)) { // percorrendo as linhas do arquivo 
  //sizeof retorn o tamanhbo do array
    acumular_conteudo_linha[strcspn(acumular_conteudo_linha, "\n")] = 0; // tirar o \n da linha para a funcao strcmp() poder comparar de forma correta.
    if (strcmp(acumular_conteudo_linha, palavra) == 0) { // comparando a palavra da linha do arquivo com a palavra fornecida
      guardar_hash(senhas_quebradas, linha1); // Palavra encontrada
    }
    n_linha = n_linha + 1;
  } 
}*/


int main(void) {
    FILE *words = fopen("palavras.txt", "r");
    if (words == NULL) {
        printf("Erro ao abrir o arquivo de palavras!\n");
        return 1;
    }

    char vetor_palavras[MAXIMO_PALAVRAS][TAMANHO_PALAVRAS] = {{""}};
    char *palavra;

    for (int i = 0; i < MAXIMO_PALAVRAS; i++) {
        palavra = ler_proxima_palavra(words);
        if (palavra != NULL) {
            strcpy(vetor_palavras[i], palavra);
        } else {
            break;
        }
    }
    fclose(words);

    int n = MAXIMO_PALAVRAS;

    gerar_combinacoes(vetor_palavras, n); //gera todas as combinacoes de senhas possiveis com base no arquivo de palavras

    FILE *hashes = fopen("hashes.txt", "a");
    if (hashes == NULL) {
        printf("Erro ao abrir o arquivo hashes.txt!\n");
        return 1;
    }

    FILE *possibilidades_senhas = fopen("todas_possibilidades.txt", "r");
    if (possibilidades_senhas == NULL) {
        printf("Erro ao abrir o arquivo de possibilidades para leitura!\n");
        fclose(hashes);
        return 1;
    }

    SHA512_CTX ctx;
    unsigned char hash[SHA512_DIGEST_LENGTH];
    char hash_hex[SHA512_DIGEST_LENGTH * 2 + 1];

    gerar_hashes(hash, hash_hex, possibilidades_senhas, &ctx, hashes);

    fclose(hashes);
    fclose(possibilidades_senhas);/*
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------//
    //OBJETIVO: COMPARAR O ARQUIVO HASHES COM O AS HASHES FORNECIDAS PELO ARQUIVO DO PROFESSOR E GUARDAR EM UM ARQUIVO CHAMADO SENHAS_QUEBRADAS.TXT
    //removendo o nome e os : do arquivo que contem as hashes a ser quebrada 
    FILE *hash_usuario_original = fopen("usuarios_senhascodificadas.txt", "r");
    FILE *hash_usuario_limpo = fopen("usuarios_senhacodificado_limpo.txt", "a");

    if (hash_usuario_original == NULL || hash_usuario_limpo == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        return 1;
    }

    //remover_nome_e_pontos(hash_usuario_original, hash_usuario_limpo);

    fclose(hash_usuario_original);
    fclose(hash_usuario_limpo);

    FILE *hashes_file = fopen("hashes.txt", "r");
    FILE *hash_usuario = fopen("usuarios_senhacodificado_limpo.txt", "r");
    if (hashes_file == NULL || hash_usuario == NULL) {
        printf("Erro ao abrir o arquivo de hashes ou o arquivo do usuário!\n");
        return 1;
    }

    FILE *senhas_quebradas = fopen("senhas_quebradas.txt", "a");
    if (senhas_quebradas == NULL) {
        printf("Erro ao abrir o arquivo senhas_quebradas.txt!\n");
        return 1;
    }

    char linha1[100], linha2[100];
    while (fgets(linha1, sizeof(linha1), hashes_file) != NULL && fgets(linha2, sizeof(linha2), hash_usuario) != NULL) {
        linha1[strcspn(linha1, "\n")] = 0;  // Remove '\n'
        linha2[strcspn(linha2, "\n")] = 0;

        if (strcmp(linha1, linha2) == 0) {
            guardar_hash(senhas_quebradas, linha1);
        }
    }

    fclose(senhas_quebradas);
    fclose(hashes_file);
    fclose(hash_usuario);*/
    //objetivo: retornar a linha do arquivo hashes que tem as hashes do arquivo senhas_quebradas.txt, essas senhas são as senhas que sao iguais a do arquivo que o professor deu
    /*
    FILE *senhas_quebradas2 = fopen("senhas_quebradas.txt", "r");
    FILE *hashes2 = fopen("hashes.txt", "r");
    FILE *linhas_hashes_iguais = fopen("linhas_hashes_iguais.txt", "a");

    if (linhas_hashes_iguais == NULL) {
        printf("Ocorreu um erro ao abri o arquivo~\n");
        return 1;
    }


    if (senhas_quebradas == NULL || hashes == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        return 1;
    }
    char linha[100],linha3[100];

    int linha_n = 0;
    
    while (fgets(linha, sizeof(linha), senhas_quebradas2) != NULL && fgets(linha3, sizeof(linha3), hashes2) != NULL) { //percorrendo o arquivo hashes e o arquivo que tem as hashes iguais as hashes dada pelo professor
        linha[strcspn(linha, "\n")];
        linha3[strcspn(linha3, "\n")]; //tirando os "\n" das linhas caso tenha

        linha_n++;

        rewind(hashes2);

        if (strcmp(linha, linha3) == 0) {
            break;  //guardando a linha do arquivo hashes.txt que contem a hash igual as hashes fornecidas pelo professor
        }
    } */
/*Preciso conseguir pegar a linha do arquivo hashes que contenha a hash igual a alguma hash do arquivo senhas_quebradas. Pois sabendo a linha do arquivo hashes que possui a mesma hash
do arquivo senhas_quebradas, posso percorrero arquivo todas_possibilidades e descobrir a senha. */
    return 0;
}

