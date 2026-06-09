# Trabalho Grau A - Documentação Técnica

## 1. Visão Geral

O `TrabalhoGrauA` é a evolução do `TrabalhoOBJ`. A proposta é exibir vários modelos 3D OBJ na mesma cena e permitir que o usuário escolha um objeto por vez para aplicar transformações geométricas diretamente nele.

Do ponto de vista técnico, o projeto resolve três problemas principais:

1. Carregamento de modelos 3D a partir de arquivos `.obj`.
2. Renderização OpenGL moderna com iluminação e shaders programáveis.
3. Controle interativo de transformação por objeto, sem afetar os demais objetos da cena.

A implementação atual foi adaptada para compilar e executar no macOS, usando GLFW, GLAD, GLM e Assimp via CMake.

## 2. Objetivo Funcional

O comportamento esperado do aplicativo é:

- carregar três modelos OBJ de forma fixa;
- exibir os modelos lado a lado na tela;
- permitir alternar o objeto selecionado;
- aplicar rotação, translação e escala somente no objeto selecionado;
- permitir navegação de câmera com teclado e mouse;
- alternar entre projeção perspectiva e ortográfica.

## 3. Estrutura Geral do Projeto

Os arquivos mais importantes do exercício ficam em `src/TrabalhoGrauA/`:

- `main.cpp`: ponto de entrada do programa, loop principal, controle de input, renderização e seleção de objeto.
- `model.hpp`: classe `Model`, responsável por carregar o OBJ via Assimp e armazenar transformações por instância.
- `mesh.hpp`: encapsula VBO, EBO e VAO de cada malha carregada.
- `shader.hpp`: compila, linka e usa shaders GLSL, além de configurar uniforms.
- `camera.hpp`: implementa a câmera em estilo FPS.
- `light.hpp` e `material.hpp`: definem a iluminação e os materiais usados no shader.
- `vertex.glsl`, `fragment.glsl`, `geometry.glsl`: shader principal da cena.
- `lightVertex.glsl`, `lightFragment.glsl`: shader usado para o cubo da luz.
- `README.md`: resumo de uso para execução rápida.

## 4. Dependências Técnicas

O projeto depende das seguintes bibliotecas:

- GLFW: criação da janela, contexto OpenGL e captura de input.
- GLAD: carregamento dinâmico de funções OpenGL.
- GLM: álgebra linear para matrizes, vetores e transformações.
- Assimp: leitura e processamento dos arquivos OBJ.
- stb_image: suporte a carregamento de imagens, mesmo que o uso atual seja limitado.

No macOS, o CMake foi configurado para:

- baixar GLFW, GLM, stb_image e Assimp automaticamente com FetchContent;
- compilar com OpenGL 4.1 Core;
- habilitar `GLFW_OPENGL_FORWARD_COMPAT` quando necessário;
- evitar conflito entre GLFW e GLAD usando `GLFW_INCLUDE_NONE`.

## 5. Arquitetura de Execução

### 5.1 Inicialização

O fluxo começa em `main()`:

1. `glfwInit()` inicializa a biblioteca GLFW.
2. São configuradas as versões do OpenGL e o perfil `CORE`.
3. A janela é criada com `glfwCreateWindow()`.
4. O contexto atual é ativado com `glfwMakeContextCurrent()`.
5. O carregador GLAD é inicializado com `gladLoadGLLoader()`.
6. Os callbacks de teclado, mouse, scroll, redimensionamento e foco são registrados.
7. A profundidade é habilitada com `glEnable(GL_DEPTH_TEST)`.
8. Os shaders são carregados a partir dos arquivos `.glsl`.
9. Os modelos OBJ são carregados e adicionados à lista de objetos da cena.

### 5.2 Loop Principal

Enquanto a janela não for fechada:

1. Calcula-se `deltaTime` com base no tempo corrente do GLFW.
2. Processam-se os inputs do usuário.
3. A cena é limpa com `glClearColor()` e `glClear()`.
4. As matrizes de projeção e visão são atualizadas.
5. Cada objeto é desenhado com sua própria transformação.
6. O cubo da luz é desenhado separadamente.
7. Os buffers são trocados com `glfwSwapBuffers()`.
8. Os eventos são processados com `glfwPollEvents()`.

## 6. Pipeline de Renderização

A renderização segue o pipeline clássico de OpenGL moderna:

### 6.1 Vertex Processing

O vertex shader recebe a posição do vértice e multiplica por:

$$
\text{gl\_Position} = P \times V \times M \times \vec{p}
$$

onde:

- `M` é a matriz do modelo;
- `V` é a matriz da câmera;
- `P` é a matriz de projeção;
- `\vec{p}` é o vértice original.

### 6.2 Fragment Processing

O fragment shader calcula a cor final com base na iluminação, material e uniformes enviados pela aplicação.

### 6.3 Geometria

O projeto possui um shader geométrico, usado no pipeline principal. Ele permite ajustes adicionais sobre a geometria antes da rasterização, dependendo da lógica definida no arquivo GLSL.

## 7. Carregamento dos Modelos OBJ

A classe `Model` encapsula o carregamento dos objetos.

### 7.1 Assimp

Em `model.hpp`, a leitura ocorre com:

- `Assimp::Importer` para abrir o arquivo;
- flags como `aiProcess_Triangulate` e `aiProcess_FlipUVs` para preparar a malha;
- travessia recursiva da árvore de nós do Assimp;
- extração de vértices, normais e índices para montar malhas renderizáveis.

### 7.2 Organização Interna

Cada `Model` mantém:

- `translate`: posição do objeto;
- `rotate`: rotação por eixo;
- `scale`: escala por eixo;
- `meshes`: lista de malhas do modelo;
- `material`: material ativo;
- `isLight`: flag para diferenciar objeto normal de objeto-luz.

Isso permite que cada instância preserve seu estado de transformação independentemente das outras.

## 8. Transformações Geométricas

O modelo final é montado nesta ordem:

1. translação;
2. rotação em X;
3. rotação em Y;
4. rotação em Z;
5. escala.

Essa ordem é importante porque transforma o sistema de coordenadas do objeto antes de chegar à projeção final.

### 8.1 Modos de Operação

O projeto suporta quatro modos de operação:

- `TRANSLATE`
- `ROTATE`
- `SCALE`
- `SCALE_SIMETRICAL`

Na prática, o uso atual do exercício ficou focado em três ações principais:

- rotacionar;
- transladar;
- escalar.

### 8.2 Controle por Eixo

O eixo ativo pode ser alterado para:

- X
- Y
- Z

Assim, a tecla de incremento/decremento atua apenas no componente selecionado da transformação.

## 9. Seleção de Objetos

A aplicação mantém os modelos em um vetor global.

Quando o usuário pressiona as teclas do teclado numérico:

1. `KP +` incrementa o índice do objeto selecionado;
2. `KP -` decrementa o índice do objeto selecionado;
3. a seleção é circular, então ao passar do último objeto o índice volta para o primeiro;
4. o novo objeto ativo é destacado visualmente.

O destaque é reforçado por:

- escala um pouco maior;
- `colorBoost` mais alto no shader.

Nesta implementação, `TAB` não está associado à seleção de objetos.

## 10. Câmera

A câmera segue o padrão de FPS.

### 10.1 Movimento

- `W`: move para frente.
- `S`: move para trás.
- `A`: move para a esquerda.
- `D`: move para a direita.

### 10.2 Mouse

O mouse altera:

- yaw;
- pitch;

Isso muda a direção do vetor `front`, o que altera a visão da câmera.

### 10.3 Scroll

O scroll ajusta o zoom da câmera.

### 10.4 Foco da Janela

A aplicação foi protegida para não capturar mouse e teclado fora de foco.

Quando a janela perde foco:

- o callback de foco marca o estado como inativo;
- os callbacks de mouse e scroll retornam imediatamente;
- o cursor volta ao modo normal;
- a captura de mouse é restaurada quando a janela volta a focar.

Esse comportamento evita interferência no uso normal do sistema operacional enquanto a aplicação permanece aberta.

## 11. Projeção

O projeto alterna entre dois tipos de projeção:

- perspectiva
- ortográfica

A variável `gPerspective` controla qual matriz é montada em tempo de execução.

### 11.1 Perspectiva

Usa `glm::perspective()` para dar profundidade visual realista.

### 11.2 Ortográfica

Usa `glm::ortho()` para uma visualização sem perspectiva.

## 12. Iluminação e Materiais

A cena possui um objeto que representa a luz.

### 12.1 Estrutura de Luz

O tipo `Light` carrega:

- posição;
- ambiente;
- difusa;
- especular;
- parâmetros de atenuação.

### 12.2 Materiais

Os modelos podem usar materiais pré-definidos, como jade e ouro.

Os materiais são enviados ao shader como uniforms e afetam a resposta visual da iluminação.

## 13. Resolução de Caminhos

O projeto usa funções auxiliares para localizar arquivos em tempo de execução.

### 13.1 OBJ

A função `resolveObjPath()` tenta localizar os `.obj` em:

- `assets/Modelos3D/`
- caminhos relativos ascendentes

### 13.2 Shaders

A função `resolveShaderPath()` tenta localizar os shaders em:

- a pasta atual;
- `src/TrabalhoGrauA/`;
- caminhos relativos ascendentes.

Isso evita falhas quando o executável é iniciado a partir de diretórios diferentes.

## 14. Controles da Aplicação

### 14.1 Entrada principal

- `ESC`: fecha o programa.
- `P`: alterna projeção.
- `KP +`: seleciona o próximo objeto.
- `KP -`: seleciona o objeto anterior.

### 14.2 Modos de transformação

- `KP 4`: rotação.
- `KP 7`: translação.
- `KP 1`: escala.
- `KP 0`: escala simétrica.

### 14.3 Eixos e ajuste

- `X`, `Y`, `Z`: escolhem o eixo ativo.
- `Q`: diminui o valor da transformação no eixo ativo.
- `E`: aumenta o valor da transformação no eixo ativo.
- `KP .`: reseta rotação e escala do objeto selecionado.

### 14.4 Navegação da câmera

- `W`, `A`, `S`, `D`: movimentação da câmera.
- Mouse: rotação da câmera.
- Scroll: zoom.

### 14.5 Iluminação e renderização

- `KP /`: alterna wireframe.
- `M`: avança para o próximo material disponível.
- `N`: retorna para o material anterior.
- `L`: aumenta a distância da luz.
- `K`: diminui a distância da luz.

### 14.6 Comportamento com foco

- Sem foco: mouse e teclado são ignorados.
- Com foco: os controles voltam a funcionar normalmente.

## 15. Pontos Importantes para a Apresentação

Se você for explicar o projeto para a professora, vale destacar estes pontos:

### 15.1 O que foi implementado

- leitura de múltiplos OBJ;
- renderização simultânea na mesma cena;
- seleção de objeto ativo;
- transformações independentes por objeto;
- câmera interativa;
- suporte a projeção ortográfica e perspectiva.

### 15.2 O que diferencia este trabalho do `TrabalhoOBJ`

- o `TrabalhoOBJ` foca principalmente em carregar e manipular OBJ com uma interação mais simples;
- o `TrabalhoGrauA` evolui isso para uma cena com vários objetos e transformações independentes;
- o pipeline usa classes mais organizadas para modelo, malha, shader, material e luz.

### 15.3 Ponto técnico mais importante

A ideia central é separar o estado de transformação de cada objeto, em vez de aplicar a transformação diretamente na cena inteira.

Isso é o que torna possível selecionar um objeto e mover apenas ele sem afetar os demais.

## 16. Limitações Atuais

- O carregamento dos modelos é hardcoded.
- O conjunto de controles foi simplificado para ficar consistente com a apresentação.
- O projeto depende de caminhos relativos e da estrutura atual do repositório.
- O foco principal é o exercício acadêmico, não uma engine genérica.

## 17. Resumo Final

O `TrabalhoGrauA` é um pequeno sistema de visualização e manipulação de objetos 3D em OpenGL moderna. Ele combina carregamento de malhas, materiais, luz, câmera e controle por teclado/mouse em uma aplicação interativa.

Do ponto de vista da disciplina, ele demonstra:

- uso de pipeline gráfico moderno;
- compreensão de matrizes de transformação;
- organização básica de cena 3D;
- interação com o usuário;
- integração com bibliotecas auxiliares como GLFW, GLAD, GLM e Assimp.
