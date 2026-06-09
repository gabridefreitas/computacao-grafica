# Trabalho Grau A

Este exercício é uma evolução do `TrabalhoOBJ`. O objetivo é carregar e renderizar vários modelos OBJ na mesma cena e permitir selecionar um objeto por vez para aplicar transformações nele.

## O que o projeto faz

- Carrega três modelos OBJ de forma fixa: `Suzanne`, `Cube` e `SuzanneSubdiv1`.
- Renderiza os objetos lado a lado na cena.
- Permite selecionar o objeto ativo de forma cíclica.
- Permite aplicar rotação, translação e escala no objeto selecionado.
- Suporta câmera livre com mouse e teclado.
- Alterna entre projeção perspectiva e ortográfica.

## Como executar

### Pelo VS Code

1. Abra a pasta raiz do repositório.
2. Execute `CMake: Configure`.
3. Execute `CMake: Build`.
4. Rode o alvo `TrabalhoGrauA`.

### Pelo terminal

```sh
cmake -S . -B build
cmake --build build --target TrabalhoGrauA
./build/TrabalhoGrauA
```

## Controles

### Janela e câmera

- `ESC`: fecha o aplicativo.
- `W`, `A`, `S`, `D`: movimentam a câmera.
- `Mouse`: gira a câmera enquanto a janela está em foco.
- `Scroll`: ajusta o zoom da câmera.
- `P`: alterna entre projeção perspectiva e ortográfica.

### Seleção de objeto

- `KP +`: seleciona o próximo objeto da lista.
- `KP -`: seleciona o objeto anterior da lista.
- A seleção é circular, então ao passar do último objeto o índice volta para o primeiro.

### Modos de transformação

- `KP 4`: ativa o modo de rotação.
- `KP 7`: ativa o modo de translação.
- `KP 1`: ativa o modo de escala.
- `KP 0`: ativa o modo de escala simétrica.

### Eixos e ajuste fino

- `X`, `Y`, `Z`: escolhem o eixo ativo.
- `Q`: diminui no eixo ativo.
- `E`: aumenta no eixo ativo.
- Setas: translação em `X` e `Y` no modo translação.
- `PageUp` / `PageDown`: translação em `Z` no modo translação.
- `KP .`: reseta rotação e escala do objeto selecionado.

### Iluminação e renderização

- `KP /`: liga e desliga wireframe.
- `M`: muda o material do objeto selecionado para o próximo da lista.
- `N`: volta o material do objeto selecionado para o anterior.
- `L`: aumenta a distância da luz.
- `K`: diminui a distância da luz.

## Observação importante

O aplicativo só processa teclado e mouse quando a janela está em foco. Quando a janela perde o foco, a captura de mouse é desativada automaticamente para não atrapalhar o uso do sistema operacional.

Observação adicional: nesta implementação, `TAB` não é um atalho ativo para seleção. A troca de objeto é feita com `KP +` e `KP -`.

## Estrutura principal

- `main.cpp`: inicialização, loop principal, entrada e renderização.
- `model.hpp`: carregamento dos OBJ e aplicação das transformações.
- `shader.hpp`: compilação e uso dos shaders.
- `vertex.glsl`, `fragment.glsl`, `geometry.glsl`: shader principal.
- `lightVertex.glsl`, `lightFragment.glsl`: shader da luz.

## Notas

- Os caminhos dos shaders são resolvidos a partir da pasta do projeto, então o executável pode ser iniciado pela raiz do repositório.
- O projeto foi ajustado para compilar no macOS com GLFW, GLAD, GLM e Assimp via CMake.
