# 02: Executar uma ação por menu e toolbar com as mesmas regras

**ID:** N02

**What to build:** Ajustar a janela de espelhamento por menu e botão, com o mesmo alvo, disponibilidade e feedback, estabelecendo um caminho compartilhado para a migração incremental.

**Blocked by:** 01 / N01 — Comparar a experiência compacta em protótipo nativo.

**Status:** in-progress

**Implementação:** piloto explicitamente autorizado pelo usuário após revisar N01: Ajustar janela usa o controlador existente por menu e botão na interface experimental. Helper passou; efeito físico, contrato read-only/transferência e migração completa ainda pendentes. N01 não foi marcado concluído.

**Origem / reconciliação:** L01, L05 e L14–L16: reutilizar controlador, transportes e guardas existentes.

## Acceptance criteria

- [ ] Ação Ajustar janela usa o controlador público existente e produz o mesmo efeito por menu e toolbar.
- [ ] Disponibilidade acompanha readiness, modo e capacidade; somente leitura não bloqueia ações de apresentação nem libera ações Android indevidamente.
- [ ] Feedback de indisponibilidade, erro e timeout permanece visível; acionamento repetido respeita a serialização existente.
- [ ] Documentar alvo selecionado versus sessão ativa e o contrato atual de somente leitura para transferência; mudanças semânticas precisam de decisão explícita.
- [ ] Extrair somente o compartilhamento necessário para essa jornada; a interface anterior continua funcionando durante a migração.

## Verification

Qt Test com helper valida efeitos, estados e equivalência das entradas; motor real valida o ajuste visível da janela.

Piloto: `compact-live-tests` valida readiness e execução por menu, com disponibilidade
igual do botão. O client real executou consulta de versão; ainda falta um telefone
para observar a mudança na janela SDL. Reutilizamos o controlador sem alterar seus transportes.
