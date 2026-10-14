# DroidCast: esteira reconciliada de interface e distribuição

Status: esteira aprovada pelo usuário, formalizada com `to-tickets` em arquivos locais.
O nome histórico deste arquivo permanece para preservar os links existentes.
IDs `N01–N18` são locais e não renumeram os tickets antigos; N15 foi desdobrado
em quatro arquivos. N17/N18 registram os esclarecimentos de oferta USB e login opt-in.
Tickets individuais têm triagem `ready-for-agent`, não execução concluída.
Data: 2026-10-13.

Especificação: [interface compacta e distribuição](compact-desktop-spec.md).

## Fontes e regras de reconciliação

- [Tickets antigos](droidcast-desktop-tickets-draft.md): critérios originais, IDs `L01–L26` neste documento.
- [Gap register](gap-register.md): estado operacional e provas ainda pendentes.
- [Histórico de implementação](implementation-progress.md): detalhes das partes 1–26.
- [Inventário scrcpy](scrcpy-feature-map.md): alcance de opções, modos e ações.
- Código atual do controlador, interface, bridge e build: desempata descrições desatualizadas.

Não tratar a tabela antiga como lista de recursos inteiramente não implementados.
O novo trabalho migra a apresentação, preserva caminhos existentes e carrega lacunas
reais até suas provas. Não fechar L01–L26 só porque a migração de UI terminou.

As obrigações antigas de preservar sete destinos/sidebar/paleta fixa são substituídas
pela direção compacta aprovada, sujeita à validação visual do protótipo. Cobertura das dezesseis
categorias significa acesso descobrível a capacidades, não dezesseis telas permanentes.
O objetivo de cobertura completa continua distinto de uma prévia distribuível.

## Cruzamento dos tickets antigos

| Origem | Estado observado / pendência | Destino da esteira nova |
| --- | --- | --- |
| L01 lifecycle | Eventos e estado implementados; hardware ainda pendente | N02/N03 preservam; N14 comprova |
| L02 stop/finalização | Bridge e resultados implementados; arquivos reais pendentes | N08 política de saída; N14 comprova |
| L03 recuperação | Caminhos e testes existentes; unplug/reconnect real pendente | N03 preserva; N14 comprova |
| L04 Wi-Fi | Parear/conectar existem; orientação e jornada real incompletas | N04 consolida fluxo; N14 comprova |
| L05 captura/APK | Alvo e falhas cobertos; prova de dispositivo/plataforma pendente | N02/N03 preservam ações; N14 comprova |
| L06 configurações | Validação, persistência e snapshots existem | N05 migra; N14 comprova configurações reais |
| L07 runtime | Fork C + server upstream compatível; build/deployment incompletos | N10 consolida proveniência e independência |
| L08 Linux | Staging não é distribuição final | N13 entrega AppImage; N14 comprova pacote |
| L09 Windows | Sem instalador e prova clean-machine | N12 entrega EXE; N14 comprova pacote |
| L10 macOS | Build/testes locais realizados; dependências privadas e assinatura pendentes | N11 entrega DMG; N14 comprova pacote |
| L11 acessibilidade | Testes existentes focam layout atual | N09 adapta convenções; N14 comprova uso real |
| L12 prévia | Gate original não concluído | N14 mantém gate de prévia, sem alegar cobertura total |
| L13 rotas | Muitas categorias implementadas; V4L2 e opções restantes faltam | N05/N06 reorganizam; N15 fecha lacunas |
| L14 toolbar Android | Ações existentes; navegação/volume usam ADB, não bridge universal | N02 preserva transportes/guardas; N14 comprova |
| L15 Android/clipboard/apps | Bridge e app picker existem; provas reais faltam | N02/N03 preservam; N14 comprova |
| L16 apresentação | Controles live existem; opções completas e flips ainda devem ser auditados | N02 preserva; N15 cobre resíduos; N14 comprova |
| L17 vídeo | Capture, display/encoder/crop existentes; codec/constraints restantes | N05 preserva; N15 fecha resíduos |
| L18 áudio | Sources/encoders/audio-only implementados; prova real e opções restantes | N05/N06 preservam; N15 audita; N14 comprova |
| L19 input | SDK/UHID/AOA, bindings e atalhos existentes; periféricos reais pendentes | N05/N09 preservam; N14 comprova |
| L20 virtual display | Produto implementado; Android 10+ real pendente | N06 preserva; N14 comprova |
| L21 câmera | Produto implementado; gates Android/capacidade e hardware pendentes | N06 reorganiza; N15 gates; N14 comprova |
| L22 OTG | Picker libusb implementado localmente; multi-device recovery/driver proof pendentes | N06 preserva discovery; N15 recovery; N14 comprova |
| L23 remoto/túneis | Transporte consistente implementado; jornadas reais pendentes | N04 preserva avançado; N14 comprova |
| L24 V4L2 | Sem produto; build ainda desabilita a saída | N15-V fecha jornada Linux; N16 exige prova se alegar cobertura completa |
| L25 recording | Formatos/migração/audio-only existentes; playback real e demais combinações pendentes | N03/N05/N06 preservam; N15 audita; N14 comprova |
| L26 cobertura completa | Gate continua aberto | N16 reconcilia auditoria final |

## Outras pendências que não desaparecem

- **Vídeo integrado:** decisão aberta, não consequência obrigatória do redesign.
  Se escolhido, criar ticket separado de frame real → input → resize → áudio/clipboard/
  gravação; não começar por reparenting exclusivo de um OS. Nenhum N abaixo promete embedding.
- **Multi-file drag/drop e progresso:** decisão de escopo ainda aberta. Preservar
  transferência explícita de um arquivo e drop da janela nativa; não inventar sucesso.
- **Presets:** possibilidade do brainstorming, não requisito confirmado nem perfil por dispositivo.
- **Perfis e sessões concorrentes:** expansão futura registrada no README; fora deste ciclo.
- **GCC 16:** workaround deve ser preservado até revalidação; não removê-lo no redesign.
- **Branding:** preservar assets; verificar seleção Desktop/Laptop nas plataformas aplicáveis.
- **Read-only e transferência:** há divergência entre o inventário antigo (bloqueia push)
  e gap register/código (permite transferência explícita). N02 deve documentar o contrato
  atual e levar qualquer mudança semântica à revisão; não resolver implicitamente ao mover menus.
- **OTG:** texto antigo ainda pede enumeration, mas a parte 26 e o diff local já a
  implementam. O resíduo é recuperação e prova hardware, não outro ticket de enumeração.
- **Build relocada:** resultado local não comprova portabilidade; toolchain temporário
  e prefixos privados continuam no risco de distribuição de N10.

## Tickets aprovados

Consulte o [índice de execução](README.md) para os 21 arquivos individuais com critérios
de aceite, bloqueios, origem e verificação. A lista abaixo mantém os IDs do plano revisto.

1. **N01 — Comparar a experiência compacta em protótipo nativo**
   - **Blocked by:** None.
   - **What it delivers:** protótipo Qt isolado, com janela de dispositivos, menus,
     modo especialista, estado de sessão e variante tray/menu bar, para avaliar o fluxo.
   - **Acceptance:** oferta USB Espelhar / Transferir arquivo… / Agora não; estados simulados identificados; modo sem tray demonstrável;
     prova macOS com SDL em foco registra acesso aos comandos e restauração; comparar
     apresentação Windows/Linux; registrar escolha visual e política de permanência.
   - **Escopo:** protótipo não troca o app nem promete implementação de todos os comandos.
     Evidência insuficiente em outra plataforma permanece pendente explícita.

2. **N02 — Executar uma ação pelo menu e toolbar com as mesmas regras**
   - **Blocked by:** N01, decisão de organização.
   - **What it delivers:** primeira ação vertical — ajustar janela ou capturar imagem —
     disponível por menu e botão, com alvo, disponibilidade, execução e feedback iguais.
   - **Acceptance:** manter o controlador como validação definitiva; fluxo real e helper
     mostram o efeito; read-only/modo/capability não divergem; decisão de alvo documentada.
     Registrar o contrato de read-only/transferência antes da migração das outras ações.
   - **Prefactoring:** extrair somente o compartilhamento necessário para essa jornada;
     manter interface antiga funcional durante migração, sem uma reescrita horizontal ampla.

3. **N03 — Espelhar e iniciar com gravação pela janela compacta**
   - **Blocked by:** N02.
   - **What it delivers:** selecionar dispositivo, iniciar, ver estado, capturar,
     encerrar e recuperar pela nova janela; menus preservam ações atuais sob demanda.
   - **Acceptance:** lifecycle real preservado; library e diagnósticos continuam
     acessíveis; modos bloqueiam ações incompatíveis; iniciar com gravação é explícito
     antes do launch e não promete início ao vivo; falhas mantêm caminho de retry.
     Não remover UI antiga antes de comprovar equivalência das jornadas migradas.

4. **N04 — Conectar por Wi-Fi sem perder rotas avançadas**
   - **Blocked by:** N03, entrada e seleção na janela compacta.
   - **What it delivers:** fluxo guiado de pareamento/conexão que termina em dispositivo
     selecionável; configurações avançadas mantêm remoto/túnel e retorno ao local.
   - **Acceptance:** portas separadas; falhas/timeout recuperáveis; códigos não persistidos;
     toda operação usa o transporte correto; uma sessão ativa mantém seu snapshot.

5. **N05 — Configurar e lançar sem perder preferências avançadas**
   - **Blocked by:** N03.
   - **What it delivers:** janela sob demanda com básicos e pesquisa avançada, provada
     pela jornada encontrar → editar → validar → salvar → lançar uma opção de vídeo.
   - **Acceptance:** categorias e flags atuais acessíveis; valores antigos e inativos
     preservados; erros direcionam ao campo; opções next-session identificadas; input,
     áudio, gravação e connection advanced seguem o mesmo contrato na migração.
     Inventariar destino de cada controle existente antes de remover a página antiga.

6. **N06 — Iniciar modos especialistas com controles contextuais**
   - **Blocked by:** N05, edição e retenção de configurações por modo.
   - **What it delivers:** seletor de modo e diálogos próprios reutilizam as jornadas
     câmera, áudio-only, virtual display e OTG já implementadas.
   - **Acceptance:** OTG abre sem ADB e usa libusb; áudio não apresenta ferramentas
     de vídeo; câmera mostra torch/zoom onde permitido; virtual display mantém app picker;
     modo inativo preserva valores; nenhum resultado simulado vira prova de hardware.
   - **Granularidade:** dividir por modo se a migração superar uma sessão de trabalho;
     cada sub-ticket deve demonstrar seleção, configuração, launch e stop completos.

7. **N07 — Acessar a sessão pelo tray/menu bar opcional**
   - **Blocked by:** N03.
   - **What it delivers:** habilitar tray, ver estado/alvo, restaurar a janela, acionar
     uma ação rápida e encerrar sessão com as mesmas regras da janela.
   - **Acceptance:** ausência/desaparecimento do tray preserva acesso pela janela;
     sem comandos inválidos para modo/read-only; stop não sai do app; nenhum espelhamento
     automático; limites de restaurar/focar SDL são demonstrados, sem global key injection.

8. **N08 — Fechar, ocultar e sair com operações em andamento**
   - **Blocked by:** N07, disponibilidade de restauração.
   - **What it delivers:** política explícita diferencia fechar janela de controle,
     fechar espelhamento, parar sessão e sair, incluindo background opt-in.
   - **Acceptance:** sem tray não ocultar a única janela recuperável; janela SDL mantém
     comportamento documentado; sair finaliza gravação e resolve transferência/cancelamento;
     timeout/forced-stop não dizem sucesso; preferências da política sobrevivem ao restart.

9. **N09 — Usar a interface com convenções de cada plataforma**
   - **Blocked by:** N03.
   - **What it delivers:** menus, teclado, foco, tema e escala funcionam na janela compacta;
     macOS posiciona comandos padrão corretamente, Windows/Linux mantêm acesso equivalente.
   - **Acceptance:** keyboard-only journey completo; nomes/estados acessíveis; comandos
     da aplicação não capturam indevidamente entrada Android; claro/escuro e escala
     testados; ajustes seguem validação/snapshots mesmo quando abertos por outros menus.

10. **N10 — Produzir um runtime independente do ambiente do desenvolvedor**
    - **Blocked by:** None.
    - **What it delivers:** pipeline fixado produz um candidato que inicia e descobre
      dispositivos fora do checkout, com motor fork, ADB e server compatíveis.
    - **Acceptance:** manifesto/revisão/checksums/licenças; bibliotecas e plugins dos
      subprocessos incluídos; não referenciar toolchain temporário ou prefixos privados;
      build limpa reproduzível; definir origem/hash do server e build Java quando houver
      divergência do upstream; USB offline funciona após instalação.

11. **N11 — Instalar e executar pelo DMG no macOS**
    - **Blocked by:** N10.
    - **What it delivers:** abrir DMG, copiar app e iniciar pelo Finder sem dependências
      de desenvolvimento, em cada arquitetura/versão declarada.
    - **Acceptance:** frameworks, Qt plugins e engine libraries resolvem no bundle;
      origem/assinatura de componentes auxiliares verificadas; assinatura/notarização
      integradas para distribuição pública; prova numa máquina limpa por arquitetura;
      dados preservados em atualização. Sem credenciais, apenas candidato não publicável.

12. **N12 — Instalar e executar pelo EXE no Windows**
    - **Blocked by:** N10.
    - **What it delivers:** instalador EXE proposto com Inno Setup instala/desinstala
      o app completo e permite iniciar uma sessão sem ferramentas Android/Qt externas.
    - **Acceptance:** DLLs/plugins incluídos; caminhos com Unicode/espaços; sem consoles
      inesperados; upgrade preserva dados; uninstall não apaga gravações; status de
      assinatura documentado; versão/arquitetura suportada verificada numa máquina limpa.

13. **N13 — Abrir o AppImage em diferentes famílias Linux**
    - **Blocked by:** N10.
    - **What it delivers:** candidato AppImage x86_64 abre e inicia uma sessão em
      ambientes representativos, com contrato de compatibilidade explícito.
    - **Acceptance:** base/glibc mínimas fixadas; Qt/SDL/FFmpeg/libusb e plugins empacotados
      conforme contrato; Debian/Ubuntu, Fedora e Arch testados; X11/Wayland e sem tray;
      requisitos de host/montagem do formato e falhas USB têm orientação; não depender
      de bibliotecas da distro de build inadvertidamente; versões fora da prova não anunciadas.

14. **N14 — Qualificar a prévia compacta multiplataforma**
    - **Blocked by:** N04, N06, N09, N11, N12, N13, N17 e N18. N08 é transitivo via N18.
    - **What it delivers:** candidatos de três plataformas com relatório de funcionalidades,
      suporte e limitações; a prévia é distinta de cobertura scrcpy completa.
    - **Acceptance:** oferta USB com envio explícito, login opt-in e saída interrompendo monitoramento; journeys reais USB/Wi-Fi, stop/recording/playback e unplug/reconnect;
      captura/APK/transferência, input/clipboard/drop, áudio e modos anunciados testados;
      remoto/túnel, FPS, branding e bridge registrados por plataforma aplicável;
      evidências ligadas a hashes/revisão; lacunas expostas no release report.
    - **Gate:** uma funcionalidade só é anunciada como validada onde há prova.
      Recurso sem prova pode constar como experimental/limitado explicitamente;
      não transformar essa declaração em encerramento dos tickets antigos completos.
      Publicar/download externo permanece ação separada.

15. **N15 — Retomar resíduos funcionais sem duplicar implementações**
    - **Natureza:** agrupamento de rastreabilidade; não publicar como uma única issue.
    - **N15-A — Configurar um parâmetro codec avançado:** bloqueado por N05.
      Entrega editor validado, persistência, argumento e sessão real; repetir em slices
      independentes para constraints/alignment e opções de apresentação encontradas na auditoria.
    - **N15-C — Explicar disponibilidade de uma configuração câmera:** bloqueado por N06.
      Entrega gate baseado em evidência de Android/capacidade, manual fallback e falha
      recuperável; metadados declarados não garantem funcionamento do HAL.
    - **N15-O — Recuperar seleção OTG entre múltiplos dispositivos:** bloqueado por N06.
      Entrega seleção determinística, serial ambíguo/ausente e refresh/reconnect sem ADB;
      hardware valida drivers/permissão; não refazer enumeration existente.
    - **N15-V — Consumir frames num dispositivo V4L2 existente:** bloqueado por N06.
      Entrega build Linux habilitada, seleção/validação de saída, launch e leitura real
      em outro aplicativo; suporte por fonte precisa de prova; não instalar módulo kernel.
      A jornada pode ser desenvolvida antes do AppImage; validar sua inclusão no pacote
      final depende de N13, sem bloquear desnecessariamente a implementação local.
    - **Demais resíduos:** cada ausência confirmada no inventário vira ticket vertical
      próprio. Escolha multi-file/progresso e embedding exige decisão de produto específica.

16. **N16 — Qualificar cobertura completa e reconciliar o registro final**
    - **Blocked by:** N14 e todos os sub-tickets N15 necessários ao escopo completo aprovado.
    - **What it delivers:** inventário relaciona cada opção/ação a destino, política,
      disponibilidade e prova; release completo não confunde reorganização com implementação.
    - **Acceptance:** L01–L26 revisados individualmente com evidência; categorias todas
      alcançáveis sem sidebar obrigatória; pendências hardware resolvidas onde se anuncia
      suporte; decisão explícita sobre embedding/multi-file; exclusões justificadas não
      são marcadas como implementadas; atualizar gap register e histórico somente por fatos.

17. **N17 — Oferecer ações ao conectar Android USB**
    - **Blocked by:** N07.
    - **What it delivers:** com app em execução, conexão USB reconhecida oferece
      Espelhar, Transferir arquivo… ou Agora não, reutilizando ações existentes.
    - **Acceptance:** uma oferta por conexão; baseline no lançamento; sem foco roubado,
      sessão/envio automático ou falso plug por Wi-Fi/ADB remoto; autorização e alvo
      revalidados; Sair encerra observação. Limite sem USB debugging é explícito.
    - **Mecanismo confirmado:** assinatura ADB por eventos, escolhida pelo usuário
      para substituir polling; recuperação limitada e nenhuma consulta periódica em repouso.

18. **N18 — Iniciar opcionalmente na sessão do usuário**
    - **Blocked by:** N08 e N17.
    - **What it delivers:** preferência opt-in para manter DroidCast disponível ao
      login, com background recuperável e fallback para janela onde não houver tray.
    - **Acceptance:** desabilitado inicialmente; registro reversível da sessão do
      usuário; sem serviço privilegiado ou USB iniciando app encerrado; single-instance;
      upgrade/uninstall respeitam registro próprio e dados; pacotes validam o caminho final.

## Ordem e fronteira de execução

Começar N01 e N10 independentemente. Após revisar o protótipo, N02 → N03 abre os
trabalhos de conexão, configurações, tray e adaptação nativa. N05 → N06 organiza
os modos; N07 → N08 fecha lifecycle de background. N10 abre os três empacotamentos
independentes. Eles podem validar o shell existente enquanto o redesign evolui;
N14 exige os artefatos do mesmo candidato final.

N07 abre N17 e N08/N17 abrem N18. Os arquivos foram renumerados em ordem de
dependências, mas IDs N originais foram preservados. Abrir arquivos/pastas Android
não faz parte do fluxo: “Transferir arquivo…” é o envio explícito já existente.

N15 continua o backlog funcional herdado em paralelo após suas dependências reais.
Não bloquear todos os empacotamentos no V4L2 nem exigir todos os recursos especialistas
para gerar um candidato testável. N16 é o gate separado de alegação de cobertura completa.

## Aprovação e formalização

O usuário aprovou esta esteira e sua fronteira de testes com “pode prosseguir assim”.
Depois confirmou o protótipo Qt isolado e pediu a criação dos documentos de planning.
Os tickets foram formalizados em arquivos separados em `docs/planning/issues`,
destino explicitamente pedido pelo usuário em lugar do padrão `.scratch` da skill.

O tracker é local neste ciclo, com triagem `ready-for-agent`; dependências continuam
impedindo o início de tickets bloqueados. N06 pode ser executado em partes por modo,
e N14 por jornada/plataforma, sem apagar o gate de integração. N15-A/C/O/V são
unidades separadas e N16 passa a listar seus bloqueios explícitos. Os esclarecimentos
posteriores do usuário acrescentam N17/N18, com monitoramento limitado ao app em
execução e inicialização opt-in; a qualificação da prévia inclui essas jornadas.

Não há nova aprovação de granularidade pendente. Revisar mudanças substanciais de
produto se aparecerem no protótipo. Issues e releases externas não foram publicadas.
