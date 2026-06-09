# Computação Gráfica - repositório de exercícios

Com base no repositório de exemplo da professora, este é o meu repositório para as atividades da disciplina no macOS 26.

## 1. Estrutura

```plaintext
computacao-grafica/
├── assets/          # Modelos 3D e texturas usados pelos exemplos
├── common/          # Código compartilhado
├── include/         # GLAD e cabeçalhos de apoio
├── src/             # Exercícios OpenGL
├── CMakeLists.txt   # Build
└── README.md
```

## 2. Ambiente no macOS 26

- CMake instalado no sistema
- Git instalado no sistema
- Xcode Command Line Tools instaladas
- VS Code com as extensões CMake Tools e C/C++
- Apple Clang como kit do projeto
- OpenGL 4.1 Core
- shaders em `#version 410 core`

## 3. Configurar no VS Code

1. Abra a pasta raiz com **File -> Open Folder**.
2. Pressione `Cmd + Shift + P`.
3. Rode `CMake: Scan for Kits`.
4. Rode `CMake: Select a Kit`.
5. Escolha **Apple Clang**.
6. Rode `CMake: Configure`.

Se o VS Code pedir o caminho do CMake, use normalmente `/opt/homebrew/bin/cmake`.

## 4. Build e execução

1. Pressione `Cmd + Shift + P`.
2. Rode `CMake: Build`.
3. Rode `CMake: Run Without Debugging`.

Se preferir terminal, o equivalente é:

```sh
cmake -S . -B build
cmake --build build
./build/Hello3D
```

## 5. Ajustes já aplicados para funcionar no Mac

Os exemplos foram ajustados para o contexto que o macOS aceita:

- `#version 410 core`
- `glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4)`
- `glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1)`
- `glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE)`
- `glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE)` no macOS

Isso vale para:

- `src/Hello3D/Hello3D.cpp`
- `src/HelloCamera/main.cpp`
- `src/HelloOBJ/main.cpp`
- `src/HelloPhong/main.cpp`

## 6. Como adicionar novos exercícios

1. Crie uma nova pasta dentro de `src/`.
2. Adicione o nome dela na lista `EXERCISES` do `CMakeLists.txt`.
3. Rode `CMake: Configure` de novo.
4. Rode `CMake: Build`.

## 7. Se der erro

Se aparecer erro de shader ou tela preta, confira primeiro:

- a versão do GLSL no arquivo do shader
- o `glfwWindowHint` do contexto OpenGL
- se o kit selecionado no VS Code é o Apple Clang
