# DroidCast: planejamento ativo

Plano aprovado pelo usuário na conversa: “pode prosseguir assim” e “cria os docs de planning”.
Especificação, fronteira de testes, granularidade e dependências aprovadas para execução
incremental. Refinamentos visuais e comportamento nativo são validados pelo protótipo;
essa aprovação não substitui provas de hardware nem publica uma release.

## Onde começar

- [Especificação aprovada](compact-desktop-spec.md): objetivos, histórias, decisões e testes.
- [Reconciliação da esteira](compact-desktop-tickets-draft.md): cruzamento completo com L01–L26 e pendências herdadas.
- [Estado implementado e lacunas](gap-register.md): referência operacional, sem confundir plano com produto pronto.
- [Histórico de implementação](implementation-progress.md): detalhes e evidências das entregas anteriores.
- [Inventário funcional scrcpy](scrcpy-feature-map.md): alcance que a nova apresentação deve preservar.

Tracker deste ciclo: arquivos locais em `docs/planning/issues`, conforme pedido do usuário.
Usamos o formato de `to-tickets`, com um arquivo por unidade, bloqueios explícitos e
`ready-for-agent`. O diretório pedido substitui o caminho padrão `.scratch` da skill.
Não existem issues externas criadas para estes tickets.

## Tickets

Os números de arquivo são a sequência local; IDs N preservam a relação com o plano
aprovado. São 21 arquivos: N15 é desdobrado em N15-A/C/O/V, e N17/N18 registram
a oferta USB e inicialização opt-in esclarecidas posteriormente pelo usuário. Todos têm
triagem `ready-for-agent`; tickets com dependências aguardam sua conclusão.

| Arquivo | Ticket | Bloqueado por | Execução |
| --- | --- | --- | --- |
| 01 | [N01 — Comparar a experiência compacta em protótipo nativo](issues/01-prototipo-nativo.md) | Nenhum | Em andamento; protótipo criado, prova SDL pendente |
| 02 | [N02 — Executar uma ação por menu e toolbar com as mesmas regras](issues/02-acoes-compartilhadas.md) | N01 | Piloto autorizado; testes passaram, prova real pendente |
| 03 | [N03 — Espelhar e iniciar com gravação pela janela compacta](issues/03-janela-compacta.md) | N02 | Piloto display com motor; migração completa pendente |
| 04 | [N04 — Conectar por Wi-Fi sem perder rotas avançadas](issues/04-conexao-wifi.md) | N03 | Não iniciado |
| 05 | [N05 — Configurar e lançar sem perder preferências avançadas](issues/05-configuracoes.md) | N03 | Não iniciado |
| 06 | [N06 — Iniciar modos especialistas com controles contextuais](issues/06-modos-especialistas.md) | N05 | Não iniciado |
| 07 | [N07 — Acessar a sessão pelo tray/menu bar opcional](issues/07-tray.md) | N03 | Piloto real; prova nativa pendente |
| 08 | [N08 — Fechar, ocultar e sair com operações em andamento](issues/08-fechar-e-sair.md) | N07 | Background/quit testados; provas reais pendentes |
| 09 | [N09 — Usar a interface com convenções de cada plataforma](issues/09-convencoes-nativas.md) | N03 | Não iniciado |
| 10 | [N10 — Produzir runtime independente do ambiente de desenvolvimento](issues/10-runtime-reproduzivel.md) | Nenhum | Não iniciado |
| 11 | [N11 — Instalar e executar pelo DMG no macOS](issues/11-macos-dmg.md) | N10 | Não iniciado |
| 12 | [N12 — Instalar e executar pelo EXE no Windows](issues/12-windows-exe.md) | N10 | Não iniciado |
| 13 | [N13 — Abrir AppImage em diferentes famílias Linux](issues/13-linux-appimage.md) | N10 | Não iniciado |
| 14 | [N17 — Oferecer ações ao conectar Android USB](issues/14-oferta-usb.md) | N07 | Eventos ADB e stream real validados; prova de plug/notificação pendente |
| 15 | [N18 — Iniciar opcionalmente na sessão do usuário](issues/15-iniciar-com-sistema.md) | N08, N17 | Não iniciado |
| 16 | [N14 — Qualificar a prévia compacta multiplataforma](issues/16-qualificacao-previa.md) | N04, N06, N09, N11, N12, N13, N17, N18 | Não iniciado |
| 17 | [N15-A — Configurar um parâmetro codec avançado](issues/17-codec-avancado.md) | N05 | Não iniciado |
| 18 | [N15-C — Explicar disponibilidade de configuração câmera](issues/18-camera-capacidade.md) | N06 | Não iniciado |
| 19 | [N15-O — Recuperar seleção OTG entre múltiplos dispositivos](issues/19-otg-recuperacao.md) | N06 | Não iniciado |
| 20 | [N15-V — Consumir frames num dispositivo V4L2 existente](issues/20-linux-v4l2.md) | N06 | Não iniciado |
| 21 | [N16 — Qualificar cobertura completa e reconciliar o registro final](issues/21-cobertura-completa.md) | N14, N15-A, N15-C, N15-O, N15-V | Não iniciado |

## Ordem de trabalho

Começar por **N01 (protótipo)** e **N10 (runtime)**, independentemente. N01 valida
as escolhas nativas antes da migração do shell. N10 viabiliza candidatos por plataforma
sem precisar esperar todas as funcionalidades ou a reorganização visual.

Depois de N03, conexão, configuração, tray e adaptação nativa podem avançar conforme
suas dependências. N14 qualifica a prévia; N16 só permite alegar cobertura completa
quando os resíduos e as provas aplicáveis estão resolvidos.

Primeira entrega N01: [protótipo Qt e roteiro de revisão](../../desktop/prototype/README.md).
Build e testes simulados aprovados no macOS; revisão nativa/SDL e demais plataformas
continuam pendentes. O usuário autorizou posteriormente conectar o piloto ao motor
real: N02/N03 têm uma primeira slice verificável, sem considerar N01 concluído ou
substituir o workspace principal. Interface experimental agora inicia em modo real;
`--demo` preserva a demonstração separada.

O usuário confirmou que o espelhamento real funcionou nesta máquina. O próximo slice
autorizado acrescenta tray/background ao piloto; N07/N08 têm testes de comportamento,
mas permanecem parciais até revisão nativa/SDL e provas reais de mídia/operações.

N17 usa eventos ADB no piloto: baseline/fila, framing, retry e ações por alvo.
Consulta periódica removida; sonda real validou assinatura sem telefone conectado.
Notificações/latência de plug e reconexão física ainda requerem prova; não tratar
como observação USB universal nem detectar sem debugging por consequência.

N07 abre N17 (oferta USB); N08 e N17 abrem N18 (login opt-in). Monitorar requer o
app em execução. A oferta usa “Transferir arquivo…” com o envio explícito existente,
sem navegação MTP. Sair encerra observação; login habilitado permite disponibilidade
na próxima sessão, não reativação ao plugar USB com o app encerrado.

## Regras de atualização

- Atualizar o estado deste índice e do arquivo individual quando o trabalho iniciar/concluir.
- Referenciar evidência e distinguir implementação automatizada de prova hardware/plataforma.
- Registrar funcionalidades/provas no gap register e histórico; não declarar implementação pelo planejamento.
- Preservar IDs L01–L26 como origem; não recriar funcionalidade já implementada.
- Resíduos adicionais descobertos na auditoria viram tickets próprios e bloqueiam N16 quando necessários.
- Publicação externa de issues ou releases é etapa separada, não executada por este planejamento.

## Histórico e decisões substituídas

A [spec antiga](droidcast-desktop-spec.md) e os [tickets antigos](droidcast-desktop-tickets-draft.md)
continuam disponíveis como histórico e critérios funcionais. A exigência antiga de
preservar sete destinos/sidebar é substituída pelo plano compacto aprovado. Reorganizar
a interface não remove o escopo scrcpy nem conclui tickets antigos automaticamente.

Decisões ainda específicas: layout final e política de background passam por N01;
versões mínimas/arquiteturas por N10–N13; vídeo integrado e multi-file permanecem
decisões de produto explícitas. Não há troca de framework prevista.
