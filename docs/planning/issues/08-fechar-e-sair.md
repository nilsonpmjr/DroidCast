# 08: Fechar, ocultar e sair com operações em andamento

**ID:** N08

**What to build:** Distinguir fechar a janela de controle, fechar o espelhamento, parar uma sessão e sair, preservando finalização e acesso ao app.

**Blocked by:** 07 / N07 — Acessar a sessão pelo tray/menu bar opcional.

**Status:** in-progress

**Implementação:** piloto real preserva sessão ao fechar para background opt-in e encerra graciosamente ao Sair. Preferências UI isoladas persistem; testes de perda de tray, round-trip e quit aguardando saída passaram. Provas de arquivo reproduzível, transferência real e desktop ainda pendentes.

**Origem / reconciliação:** L02 e gravação/transferência atuais; novo lifecycle de background.

## Acceptance criteria

- [ ] Permanência em segundo plano é opt-in; lançamento manual abre janela e não cria sessão automaticamente. Inicialização de login é entregue em N18, com fallback recuperável sem tray.
- [ ] Sair encerra o monitoramento e processos próprios; conectar USB com DroidCast encerrado não reabre o app. Ocultar a janela com background habilitado mantém a observação.
- [ ] Sem acesso comprovado de restauração, fechar não oculta a única janela recuperável.
- [ ] Comportamento de fechamento da janela SDL é explicitado separadamente do controle Qt.
- [ ] Sair durante gravação usa parada graciosa e resultado de finalização; timeout/kill não indicam sucesso.
- [ ] Sair durante transferência resolve conclusão ou cancelamento com feedback explícito.
- [ ] Preferência de background persiste; fechar/reabrir/retry não cria subprocessos órfãos.

## Verification

Helper cobre transições e timeout; gravação reproduzível e fechamento das janelas reais validam integração.
