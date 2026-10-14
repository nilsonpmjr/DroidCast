# 07: Acessar a sessão pelo tray/menu bar opcional

**ID:** N07

**What to build:** Habilitar acesso rápido, consultar dispositivo/estado, restaurar a janela e encerrar a sessão pelo tray/menu bar.

**Blocked by:** 03 / N03 — Espelhar e iniciar com gravação pela janela compacta.

**Status:** in-progress

**Implementação:** piloto real com tray opt-in, estado/alvo, restauração Qt e comandos compartilhados. Testes de disponibilidade/fechar/stop/quit passaram; revisão nativa com SDL em foco e prova por plataforma continuam pendentes. N03 permanece parcialmente migrado.

**Origem / reconciliação:** Novo acesso às ações existentes de L01/L14–L16; não adiciona sessão concorrente.

## Acceptance criteria

- [ ] Transferir arquivo… abre o fluxo explícito existente com alvo/arquivo/destino e feedback; não promete abrir a pasta Android. A oferta automática por plug é entregue separadamente em N17.
- [ ] Tray é opcional e sua disponibilidade é detectada; a janela permite executar todas as tarefas essenciais.
- [ ] Mostrar janela, estado da sessão e pelo menos uma ação rápida usam os mesmos contratos da janela compacta.
- [ ] Disponibilidade respeita modo/read-only/capacidades; encerramento da sessão não equivale a sair do app.
- [ ] Ausência ou desaparecimento do tray não deixa o aplicativo inacessível.
- [ ] Não autoespelhar nem sintetizar teclas globais para controlar/focar SDL; restauração suportada é comprovada e limitações explicitadas.

## Verification

Comportamento observável com e sem tray; prova por desktop real para menu e restauração.

Critérios automatizados cobertos com helper e disponibilidade do tray injetada;
não interpretar como prova do ícone do sistema. Comandos não usam teclado global.
Restaurar significa abrir a janela Qt, não prometer foco na SDL.
