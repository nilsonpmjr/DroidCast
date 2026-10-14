# 09: Usar a interface com convenções de cada plataforma

**ID:** N09

**What to build:** Usar os mesmos recursos com menus, teclado, tema, escala e acessibilidade adequados ao macOS, Windows e Linux.

**Blocked by:** 03 / N03 — Espelhar e iniciar com gravação pela janela compacta.

**Status:** ready-for-agent

**Implementação:** não iniciada. O rótulo de triagem não significa que os bloqueios foram concluídos.

**Origem / reconciliação:** L11/L19; adapta apresentação sem reduzir a abrangência funcional.

## Acceptance criteria

- [ ] macOS posiciona Configurações/Sair no menu apropriado; Windows/Linux têm acesso funcional equivalente.
- [ ] Jornada selecionar → iniciar → agir → encerrar é operável por teclado, com foco visível e nomes acessíveis.
- [ ] Atalhos da aplicação e entrada Android não conflitam silenciosamente quando SDL tem foco.
- [ ] Tema claro/escuro, escala elevada e tamanho mínimo definido no protótipo mantêm conteúdo alcançável.
- [ ] Menús e diálogos preservam validação e snapshot; sucesso não depende de contagem de widgets ou pixels.

## Verification

Qt Test cobre jornadas; inspeção nativa valida menus, foco, tecnologias assistivas e escala.
