# Trabalho de Estrutura de Dados: Implementações em Disco

Este documento descreve um projeto acadêmico que implementa e analisa três diferentes estruturas de dados para armazenamento e gerenciamento de registros em disco: **Árvore B+**, **Tabela Hash com Endereçamento Aberto** e **Heap Binário (Max-Heap)**. O objetivo é explorar e comparar as características de performance de cada abordagem para operações de inserção, busca e remoção.

O projeto foi desenvolvido em C e cada estrutura de dados está contida em seu próprio módulo.

---

## 1. Estrutura Geral do Projeto

O repositório está organizado da seguinte forma, com cada estrutura de dados em seu próprio diretório:

- **`/ (diretório raiz do projeto)`**
  - `README.md`
  - **`B+/`**
    - `bMais.c`, `bMais.h`, `bMais.exe`
    - `meta.dat`, `indices.idx`, `folhas.dat`
  - **`hash/`**
    - `hash.c`, `hash.h`, `hash.exe`
    - `hash.dat`
  - **`heap/`**
    - `heap.c`, `heap.h`, `heap.exe`
    - `heap.dat`
  - **`registros/`**
    - `gerar.c`, `gerar.h`, `gerar.exe`
    - `registros.dat`
  - `outros arquivos de configuração (.gitignore, etc.)`

---
- **`B+/`**: Contém a implementação da Árvore B+.
- **`hash/`**: Contém a implementação da Tabela Hash.
- **`heap/`**: Contém a implementação do Heap Binário.
- **`registros/`**: Contém um utilitário para gerar o arquivo de dados (`registros.dat`) que é usado para popular as estruturas.

---

## 2. Utilitário: Gerador de Registros

Para popular e testar as estruturas de dados implementadas, foi criado um programa utilitário localizado na pasta `registros/`. Este programa é responsável por gerar um arquivo binário (`registros.dat`) contendo uma grande quantidade de registros de alunos com dados aleatórios.

### 2.1. Propósito

O objetivo do `gerar.c` é fornecer uma base de dados consistente e de volume considerável para que as operações de carga inicial da **Árvore B+**, da **Tabela Hash** e do **Heap** possam ser executadas. Isso permite testar o desempenho e a corretude das estruturas com uma quantidade significativa de dados.

### 2.2. Funcionamento

O programa gera 10.000 registros, cada um contendo um nome, um CPF e uma nota gerados aleatoriamente:

-   **Nome (`gerarNome`):** Um nome aleatório de 6 a 10 caracteres é criado, alternando entre consoantes e vogais para gerar uma sequência pronunciável e de aparência mais natural.
-   **CPF (`gerarCPF`):** Um número de CPF é gerado como uma string de 11 dígitos aleatórios. Não há garantia de unicidade, mas a aleatoriedade torna a ocorrência de duplicatas rara em um conjunto de 10.000 registros.
-   **Nota:** Uma nota de 0 a 100 é atribuída aleatoriamente.

Ao final da execução, o arquivo `registros/registros.dat` é criado ou sobrescrito com os novos dados.

### 2.3. Como Usar

O gerador pode ser compilado e executado de forma independente.

1.  **Compilação:**
    ```bash
    gcc -std=c11 registros/gerar.c -o registros/gerar.exe
    ```

2.  **Execução:**
    ```bash
    ./registros/gerar.exe
    ```
    Após a execução, o arquivo `registros/registros.dat` estará pronto para ser consumido pelos programas principais do projeto.

---

## 3. Estruturas de Dados Implementadas

A seguir, um detalhamento técnico de cada uma das estruturas de dados desenvolvidas.

### 3.1. Árvore B+

A Árvore B+ é uma estrutura de dados em árvore auto-balanceada, ideal para sistemas de gerenciamento de banco de dados e sistemas de arquivos, onde os dados são armazenados em disco.

#### 3.1.1. Arquitetura de Arquivos Adotada
A implementação utiliza um modelo de 3 arquivos para garantir a persistência e a organização dos dados:
- **`meta.dat`**: Armazena metadados da árvore, como a posição (`offset`) do nó raiz e um flag indicando se a raiz é uma folha. Isso permite que a árvore cresça e encolha em altura dinamicamente.
- **`indices.idx`**: Armazena exclusivamente os **nós internos**, que contêm apenas chaves de roteamento para guiar a busca.
- **`folhas.dat`**: Armazena exclusivamente os **nós folha**. Nesta implementação, os dados (`Registros`) estão **embutidos diretamente** nos nós folha, e as folhas são interligadas para permitir varreduras sequenciais.

#### 3.1.2. Estruturas de Dados Principais
```c
// Metadados da árvore: aponta para a raiz atual
typedef struct {
    long pos_raiz;
    int raiz_eh_folha;
} Metadados;

// Nó para os níveis de índice da árvore
typedef struct {
    // Flag essencial para saber se os filhos são folhas ou outros nós internos.
    int aponta_para_folha; 
    int num_chaves;
    int chaves[MAX_CHAVES];
    long ponteiros_nos[MAX_CHAVES + 1];
} NoInterno;

// Nó para o nível de dados da árvore (folhas)
typedef struct {
    int num_chaves;
    int chaves[MAX_CHAVES];
    // Os dados são embutidos diretamente no nó.
    Registro dados[MAX_CHAVES]; 
    // Ponteiro para a próxima folha, formando uma lista encadeada.
    long proxima_folha; 
} NoFolha;
```
#### 3.1.3. Lógica das Operações Fundamentais
- **Busca (`buscar`):** Navegação *top-down* que começa na raiz (localizada via `meta.dat`), percorre os nós de índice e termina em um nó folha, onde o dado é encontrado.
- **Inserção (`inserir`):** Ocorre sempre em um nó folha. Um nó cheio causa um **split** (divisão), promovendo uma chave ao nó pai, podendo propagar a operação até a raiz e aumentar a altura da árvore.
- **Remoção (`remover`):** Ocorre em um nó folha. Se a remoção causa um **underflow** (nó com poucas chaves), o problema é corrigido por **redistribuição** (empréstimo de um irmão) ou **fusão** (merge com um irmão). A fusão pode propagar o underflow para os níveis superiores, podendo diminuir a altura da árvore.

---

### 3.2. Tabela Hash com Endereçamento Aberto

Este módulo implementa uma Tabela Hash para acesso direto aos registros. A abordagem utiliza uma técnica de **Hashing Estático**, onde o espaço de armazenamento é pré-alocado, com um mecanismo de **Endereçamento Aberto** para resolver colisões.

#### 3.2.1. Arquitetura e Técnica de Colisão
- **Arquivo Único (`hash.dat`):** Toda a tabela hash (um array de `Registros` de tamanho fixo `TAM`) é armazenada neste arquivo.
- **Tratamento de Colisão (Sondagem Linear):** Se a posição calculada pela função de hash já estiver ocupada, o algoritmo simplesmente testa a próxima posição sequencial (`p+1`, `p+2`, etc.) até encontrar um espaço livre.
- **Tratamento de Remoção (Tombstone):** Para não quebrar as cadeias de sondagem, um registro removido tem seu estado alterado para `REMOVIDO`, servindo como um marcador que permite que as buscas continuem passando por aquele slot.

#### 3.2.2. Estruturas de Dados Principais
```c
// Enum para controlar o estado de cada posição na tabela hash
typedef enum Estado { 
    LIVRE,   // Posição disponível.
    OCUPADO, // Posição contém um registro válido.
    REMOVIDO // Posição continha um registro que foi removido.
} Estado;

// O próprio Registro armazena seu estado.
typedef struct {
    char nome[51];
    char cpf[12];
    float nota;
    Estado estado;
} Registro;
```
#### 3.2.3. Lógica das Operações Fundamentais
- **Inserção (`inserir`):** Calcula o hash, procura um slot `LIVRE` ou `REMOVIDO` usando sondagem linear e insere o novo registro.
- **Busca (`consultar`):** Calcula o hash e segue a cadeia de sondagem linear (passando por slots `OCUPADO`s e `REMOVIDO`s) até encontrar a chave ou um slot `LIVRE`.
- **Remoção (`remover`):** Encontra o registro e altera seu estado para `REMOVIDO`.

---

### 3.3. Heap Binário (Max-Heap)

Este módulo implementa uma Fila de Prioridades utilizando a estrutura de dados **Max-Heap**, otimizada para encontrar e remover rapidamente o registro com a maior `nota`.

#### 3.3.1. Conceito e Implementação em Disco
- **Estrutura de Max-Heap:** O Heap é uma árvore binária quase completa onde o valor de um nó pai é sempre maior ou igual ao de seus filhos, garantindo que o registro de maior `nota` esteja sempre na raiz.
- **Representação em Array (em Disco):** A árvore é representada implicitamente pela ordem dos registros no arquivo `heap.dat`, que funciona como um array. A navegação (pai, filhos) é feita com cálculos de índice.

#### 3.3.2. Estrutura de Dados Principal
```c
// A struct base usada para os elementos do Heap
typedef struct {
    char nome[51];
    char cpf[12];
    float nota;
} Registro;
```
#### 3.3.3. Lógica das Operações Fundamentais
- **Inserção (`inserir`):** Adiciona o novo registro no final do arquivo e aplica a rotina `subir` (*bubble-up*), trocando o elemento com seu pai sucessivamente até que a propriedade de Max-Heap seja restaurada.
- **Remoção do Topo (`removerTopo`):** Substitui o registro da raiz pelo último registro do arquivo, diminui o tamanho do heap e aplica a rotina `descer` (*sift-down*) na nova raiz, trocando-a por seu maior filho até restaurar a propriedade de Max-Heap.
- **Consulta ao Topo (`consultarTopo`):** Operação O(1) em lógica que apenas lê e exibe o primeiro registro do arquivo.