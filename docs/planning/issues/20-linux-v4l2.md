# 20: Consumir frames num dispositivo V4L2 existente

**ID:** N15-V

**What to build:** Selecionar uma saída Linux virtual existente, iniciar fonte compatível e consumir frames reais em outro aplicativo.

**Blocked by:** 06 / N06 — Iniciar modos especialistas com controles contextuais.

**Status:** ready-for-agent

**Implementação:** não iniciada. O rótulo de triagem não significa que os bloqueios foram concluídos.

**Origem / reconciliação:** L24: lacuna funcional com V4L2 desabilitado no build atual.

## Acceptance criteria

- [ ] Build Linux habilita e verifica a capability do motor; não apresenta opção funcional em builds sem suporte.
- [ ] Selecionar/validar dispositivo existente e configurar buffering termina em frames reais consumidos externamente.
- [ ] Prérequisito/dispositivo ausente e plataforma incompatível têm feedback claro.
- [ ] Display é provado; câmera só é anunciada se houver prova própria; Android/input/read-only continuam corretos.
- [ ] Não instalar módulo kernel/alterar permissões automaticamente; registrar orientação dos requisitos.
- [ ] Desenvolvimento local independe do AppImage; verificação de inclusão no pacote exige N13.

## Verification

Consumidor externo valida frames; helper cobre indisponibilidade. Pacote Linux final repete a jornada quando pronto.
