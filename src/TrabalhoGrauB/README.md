# Trabalho de Renderização de Cenas do Grau B
## Christian Aguiar Plentz, Gabriel de Freitas Adolfo, Bruno Silveira da Silva

Para compilar o programa basta rodar o comando cmake -S . -B build/ para gerar o arquivo Makefile e compilar utilizando o comando make

O programa utiliza caminhos de arquivo da pasta incial do projeto então ao rodar o executavel faça questão do shell estar localizado nela.

O Programa utilzia o arquivo scene.json para definir os objetos e suas curvas paramétricas.

## Controles

Os controles são todos utilizando o Teclado Numerico em um teclado 100% então São esses:

/ (Ativa/Desativa o desenho de wireframe sobre o mesh)

\- + (Escolhe o Modelo antecessor/sucessor da lista de modelos carregados e seta o tipo de Operação para Translação Automaticamente)

7 (Ativa Modo de Translação do Modelo) 8 9 (Diminui e Adiciona X respectivamente) 

4 (Ativa Modo de Rotação do Modelo)    5 6 (Diminui e Adiciona Y respectivamente)

1 (Ativa Modo de Escala do Modelo)     2 3 (Diminui e Adiciona Z respectivamente)

0 (Ativa o Modo de Escala Simetrica do Modelo, Se adicionar ou diminuir qualquer eixo vai aumentar/diminuir todos igualmente)

, (Teclado ABNT2) ou . (Teclado Internaciona) (Botão Del do NumPad) reinicia os Valores Padrões do Modelo atual da Rotação e Escala

NM Muda o material do Objeto atualmente em seleção
