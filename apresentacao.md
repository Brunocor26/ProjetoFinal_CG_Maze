# Apresentação do Projeto: Jogo de Labirinto 3D com OpenGL

## 1. Capa / Título
*   **Título:** Jogo de Labirinto 3D Procedural
*   **Subtítulo:** Computação Gráfica e Ambientes Virtuais
*   **Autores:** Bruno Correia, Henrique Laia
*   **Tecnologia:** C++ / OpenGL (Moderno 3.3+)
*   **Resumo:** Exploração de um labirinto gerado aleatoriamente com iluminação dinâmica e componentes multijogador.

---

## 2. Objetivos e Motivação
*   **Objetivo Principal:** Desenvolver um motor gráfico 3D de raiz utilizando OpenGL.
*   **Motivação:** Aplicar conceitos teóricos de computação gráfica (Shaders, Transformações, Iluminação) num cenário prático e interativo "First-Person".
*   **Desafios Propostos:**
    *   Não usar motores de jogo prontos (Unity/Unreal).
    *   Gerar geometria dinamicamente (não apenas carregar modelos).
    *   Implementar comunicação em rede (Sockets).

---

## 3. Tecnologias Utilizadas (A "Stack")
*   **Linguagem:** C++ (Standard 17)
*   **API Gráfica:** OpenGL 3.3 Core Profile
*   **Janelas e Input:** GLFW
*   **Carregamento de Extensões:** GLAD
*   **Matemática:** GLM (OpenGL Mathematics) - essencial para matrizes e vetores.
*   **Texto:** FreeType (renderização de fontes TrueType).
*   **Imagens:** STB Image (carregamento de texturas PNG/JPG).
*   **Build System:** CMake.

---

## 4. Arquitetura do Sistema
Uma visão geral de como o código está organizado:

1.  **Game Class (`Game.cpp`):** O "cérebro" do jogo. Gere o loop principal, inputs, e chama os métodos de renderização.
2.  **Maze Class:** Responsável pela lógica da grelha e geração da malha (mesh) das paredes.
3.  **Shader Class:** Abstração para compilar, linkar e ativar programas GLSL.
4.  **Camera System:** Câmara FPS (Euler Angles: Pitch/Yaw) que processa movimento e visão com o rato.
5.  **Network Module:** Gestão de sockets TCP para comunicação Host/Cliente.

---

## 5. Implementação: Geração do Labirinto
*   **Algoritmo:** Kruskal (Randomized).
    *   Garante um labirinto "perfeito" (unicursal, sem loops, sempre com solução).
    *   Começa com todas as células isoladas (paredes em tudo).
    *   Remove paredes aleatoriamente ligando conjuntos disjuntos até que todas as células façam parte do mesmo conjunto.
*   **Do Algoritmo para o 3D:**
    *   A classe `Maze` traduz a grelha de `int` (0=Parede, 1=Caminho) para vértices.
    *   Cada "Parede" é instanciada visualmente usando um cubo padrão, texturizado com mapas de normais (Bump mapping).

---

## 6. Pipeline de Renderização e Shaders
Utilizamos Shaders GLSL personalizados para controlar o visual:

*   **Modelo de Iluminação:** Blinn-Phong.
    *   Utiliza o vetor *Halfway* para cálculos especulares mais eficientes e suaves que o Phong tradicional.
*   **Iluminação Dinâmica (Lanterna):**
    *   Um **Spotlight** acoplado à câmara.
    *   Cálculo de atenuação quadrática (a luz enfraquece com a distância).
    *   Suavização das bordas do cone de luz (`cutOff` vs `outerCutOff`).
*   **Materiais PBR (Simplificado):**
    *   Uso de texturas Diffuse (Cor), Specular (Brilho) e Normal Maps (Relevo) nas paredes e chão.

---

## 7. Efeitos Visuais Avançados
*   **Fog (Nevoeiro):**
    *   Cálculo exponencial da distância do fragmento à câmara.
    *   Mistura (`mix`) suave entre a cor do objeto e a cor de fundo (preto/cinzento), escondendo o "fim do mundo" e aumentando a atmosfera de mistério.
*   **Portal "Mercúrio Líquido" (Shader Procedural):**
    *   Não usa texturas estáticas.
    *   O shader calcula ondas sinusoidais baseadas no tempo (`sin(time)`).
    *   Efeito **Fresnel**: As bordas brilham mais intensamente para simular energia ou metal líquido.

---

## 8. Funcionalidades de Rede (Multiplayer)
*   **Arquitetura:** Cliente-Servidor (Host-Client).
*   **Protocolo:** TCP (Sockets POSIX/Winsock).
*   **Mecânica:**
    *   **Host:** Joga livremente. Quando atinge o portal, envia comando `UNLOCK`.
    *   **Client:** Começa preso. Recebe `UNLOCK` e ganha liberdade para terminar o nível.
    *   Sincronização de cor do ambiente (`inheritedColor`) enviada pela rede.

---

## 9. Interface (HUD) e Minimapa
*   **Minimapa:**
    *   Não é uma segunda câmara (seria pesado renderizar a cena 2 vezes).
    *   É uma representação 2D desenhada diretamente sobre o ecrã.
    *   Itera sobre a matriz lógica do labirinto e desenha quadrados 2D (Shader simples `simple.frag`).
*   **Texto:** Renderização de glifos carregados via FreeType e desenhados como quads texturizados.

---

## 10. Q&A - Perguntas Frequentes (Para Defesa)

### P1: Que shaders são usados no projeto?
**R:** Usamos principalmente dois programas de shader:
1.  **`blinn_phong.vert` / `.frag`:** O shader principal. Trata de toda a geometria 3D, texturas, iluminação Blinn-Phong, lanterna, nevoeiro e o efeito especial do Portal.
2.  **`simple.vert` / `.frag`:** Um shader auxiliar extremamente leve, usado apenas para desenhar cores sólidas 2D. Usado para o Minimapa e para os fundos de texto/overlays.

### P2: Como é gerado o labirinto e passado para o OpenGL?
**R:**
1.  **Lógica:** O algoritmo de Kruskal corre no CPU e preenche uma matriz 2D (`int grid[][]`) com 0s e 1s.
2.  **Geometria:** A classe `Maze` lê essa matriz. Para cada célula "Parede", não criamos vértices novos; usamos uma técnica chamada **Instancing** (ou desenhar repetidamente o mesmo cubo).
3.  **Desenho:** Temos um único VBO que define a forma de UM cubo. No loop de renderização (`Game::Render`), desenhamos esse cubo várias vezes, atualizando a matriz `model` (transformação) para a posição correta de cada parede da grelha (x, z).

### P3: Porque escolheram Blinn-Phong em vez de Phong?
**R:** O modelo de Blinn-Phong utiliza o vetor "Halfway" (a média entre a direção da luz e a direção da visão) para calcular o reflexo especular. Computacionalmente é mais eficiente que calcular o vetor de reflexão exato (necessário no Phong) e produz brilhos especulares visualmente mais realistas e menos "artificiais" quando o ângulo de luz é rasante.

### P4: Como funciona a colisão com as paredes?
**R:** Utilizamos deteção de colisão do tipo **AABB (Axis-Aligned Bounding Box)** simplificada.
Como o labirinto é uma grelha perfeita, sabemos exatamente que coordenadas (x, z) correspondem a uma parede.
Quando o jogador tenta mover-se para uma posição `(x, z)`, convertemos essa posição flutuante para índices da matriz da grelha `[gridX][gridZ]`. Se essa célula for uma parede (valor 0), o movimento é bloqueado. Adicionámos também um pequeno "raio" ao jogador para impedir que a câmara atravesse esquinas ("clipping").

### P5: Como é feito o efeito do Portal? É uma textura?
**R:** Não é uma textura de imagem. É um efeito **procedural** calculado em tempo real no Fragment Shader.
Usamos funções matemáticas (`sin`, `cos`) combinadas com a variável `uniform time` para criar ondas que se movem pela superfície da esfera.
Adicionalmente, aplicamos um cálculo de **Fresnel** (`dot(viewDir, normal)`), que faz com que as bordas da esfera brilhem mais que o centro, dando o aspeto volumétrico e etéreo.

### P6: O jogo suporta modelos 3D externos?
**R:** Sim. Embora as paredes sejam cubos gerados por código, implementámos um carregador de ficheiros `.OBJ` (usando a biblioteca `tiny_obj_loader`). Isto é utilizado para carregar as árvores (`Tree.obj`) que decoram o ambiente exterior do labirinto.

### P7: Como funciona o Networking? Há latência?
**R:** Usamos Sockets TCP bloqueantes com verificação não-bloqueante (`select()`) no loop do jogo, para não congelar a renderização.
O TCP garante que os pacotes importantes (como o sinal de `UNLOCK`) chegam obrigatoriamente e na ordem certa, o que é crítico para a lógica do jogo. Dado que enviamos pouca informação (apenas estados e cores, não posições contínuas em tempo real), a latência é impercetível para este caso de uso.
