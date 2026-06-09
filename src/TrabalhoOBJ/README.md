# Selecionando e aplicando transformações em objetos

O objetivo deste exercício é permitir a exibição de um ou mais modelos 3D na cena, seleção e aplicação de transformações nos mesmos.

## Descrição da Tarefa

Para isso, você vai fazer um programa com OpenGL moderna que atenda os seguintes requisitos:

1. Leitura de arquivos .obj (recomenda-se testar com os modelos que estão em nosso repositório de exemplos da disciplina) – pode ser um carregamento hardcoded de 2 a 4 objetos diferentes.
2. Exibir mais de um objeto na tela (organizar eles na tela, como uma lista ou “tabela”).
3. Permitir a seleção dos objetos, a partir de uma tecla (inicia selecionado o primeiro objeto, e ao apertar a tecla o próximo objeto do array ou lista é selecionado, de forma cíclica).
4. Permitir a aplicação de transformações no objeto selecionado:
    1. Rotacionar (R) nos eixos x, y e z
    2. Transladar (T) nos eixos x, y e z (pode-se usar as setas ou WASD)
    3. Aplicar escala (S) nos eixos x, y e z (pode-se ter a opção de escala uniforme também)

## Status da Implementação

Implementação inicial criada em `main.cpp` com:

- Carregamento hardcoded de 3 modelos OBJ (`Suzanne`, `Cube`, `SuzanneSubdiv1`);
- Renderização simultânea dos objetos com `VAO` e `glDrawArrays` por instância;
- Seleção cíclica do objeto ativo;
- Aplicação de rotação, translação e escala somente no objeto selecionado.

## Mapeamento de Teclas (versão atual)

- `TAB`: seleciona o próximo objeto (cíclico)
- `R`: modo de rotação
- `T`: modo de translação
- `S`: modo de escala
- `X`, `Y`, `Z`: eixo ativo da transformação
- `Q` / `E`: decrementa / incrementa no eixo ativo (somente modos rotação e escala)
- `Setas` + `PageUp/PageDown`: translação em X/Y/Z (modo translação)
- `W`, `A`, `S`, `D`: navegação da câmera
- `P`: alterna entre projeção perspectiva e ortográfica

### Simplificações aplicadas

- Removido comando redundante de translação por `Q/E` no modo `T`;
- Removido comando de escala uniforme (`U/J`) para reduzir quantidade de atalhos;
- Corrigida a direção horizontal para ficar intuitiva na tela (`Seta Esquerda` move para esquerda e `Seta Direita` move para direita);
- Realce do objeto selecionado reforçado (maior contraste e escala).

## Cenários de Teste

### CT-01: Inicialização e carregamento de modelos

**Objetivo:** validar que 3 objetos são carregados e renderizados na abertura.

**Passos:**
1. Executar `build/TrabalhoGrauA`.
2. Observar o terminal na inicialização.
3. Observar a cena logo após abrir a janela.

**Resultado esperado:**
- O terminal exibe 3 mensagens `Objeto carregado` (Suzanne, Cube e SuzanneSubdiv1).
- A cena mostra 3 objetos lado a lado.
- Um objeto inicia selecionado com destaque visual.

### CT-02: Seleção cíclica de objeto

**Objetivo:** validar a troca de seleção em loop.

**Passos:**
1. Pressionar `TAB` uma vez.
2. Pressionar `TAB` novamente.
3. Pressionar `TAB` uma terceira vez.

**Resultado esperado:**
- O destaque muda para o próximo objeto a cada `TAB`.
- Após o último objeto, a seleção volta para o primeiro.
- O terminal atualiza `Selecionado: [indice] Nome | Modo: ... | Eixo: ...`.

### CT-03: Rotação no objeto selecionado

**Objetivo:** validar rotação por modo/eixo sem afetar os demais objetos.

**Passos:**
1. Selecionar um objeto com `TAB`.
2. Pressionar `R` (modo rotação).
3. Pressionar `X` e manter `E` por 1 a 2 segundos.
4. Repetir com `Y` e depois `Z`.

**Resultado esperado:**
- Apenas o objeto selecionado gira.
- O eixo de rotação muda conforme `X`, `Y`, `Z`.
- Os demais objetos permanecem inalterados.

### CT-04: Translação no objeto selecionado

**Objetivo:** validar deslocamento em todos os eixos.

**Passos:**
1. Selecionar um objeto.
2. Pressionar `T` (modo translação).
3. Usar `Q/E` com eixo `X`, depois `Y`, depois `Z`.
4. Usar setas e `PageUp/PageDown` para deslocamento alternativo.

**Resultado esperado:**
- Apenas o objeto selecionado se move.
- O objeto se desloca corretamente em X, Y e Z.
- Não ocorre salto abrupto nem movimentação dos outros objetos.

### CT-05: Escala por eixo e uniforme

**Objetivo:** validar escala por eixo e escala uniforme com limite mínimo.

**Passos:**
1. Selecionar um objeto.
2. Pressionar `S` (modo escala).
3. Com eixo `X`, usar `E` para aumentar e `Q` para reduzir.
4. Repetir para `Y` e `Z`.
5. Usar `U` para escala uniforme aumentar e `J` para reduzir.

**Resultado esperado:**
- Apenas o objeto selecionado muda de tamanho.
- Escala por eixo altera somente o eixo ativo.
- Escala uniforme altera X/Y/Z juntos.
- O objeto não colapsa (escala mínima respeitada).

### CT-06: Independência entre objetos

**Objetivo:** comprovar que transformações são persistentes por objeto.

**Passos:**
1. Selecionar objeto A e aplicar rotação/translação/escala.
2. Trocar para objeto B com `TAB` e aplicar transformações diferentes.
3. Voltar para objeto A com `TAB`.

**Resultado esperado:**
- Cada objeto mantém seu próprio estado de transformação.
- O estado do objeto A não é perdido após editar B.

### CT-07: Projeção e navegação da câmera

**Objetivo:** validar suporte de câmera e projeção sem quebrar as transformações.

**Passos:**
1. Mover a câmera com `W`, `A`, `S`, `D`.
2. Alternar projeção com `P` algumas vezes.
3. Aplicar uma transformação em um objeto após alternar projeção.

**Resultado esperado:**
- A câmera navega normalmente.
- Alternância de projeção funciona sem travamentos.
- Transformações continuam aplicadas no objeto selecionado.

### CT-08: Encerramento

**Objetivo:** validar fechamento limpo da aplicação.

**Passos:**
1. Pressionar `ESC`.

**Resultado esperado:**
- Janela fecha imediatamente.
- Processo encerra sem erro visível no terminal.

## Próximos Ajustes Sugeridos

- Ajustar sensibilidade dos incrementos de transformação para apresentação final;
- Refinar o destaque visual do objeto selecionado;
- Se necessário, alinhar o mapeamento exatamente ao que o PDF exigir.