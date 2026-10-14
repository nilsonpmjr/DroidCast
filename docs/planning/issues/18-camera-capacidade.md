# 18: Explicar disponibilidade de configuração câmera

**ID:** N15-C

**What to build:** Conhecer a disponibilidade de uma configuração de câmera pela evidência Android/capacidade e recuperar falhas sem perder a entrada manual.

**Blocked by:** 06 / N06 — Iniciar modos especialistas com controles contextuais.

**Status:** ready-for-agent

**Implementação:** não iniciada. O rótulo de triagem não significa que os bloqueios foram concluídos.

**Origem / reconciliação:** L21 e Camera gap; não reimplementa captura/torch/zoom existentes.

## Acceptance criteria

- [ ] Selecionar uma configuração de câmera e demonstrar disponibilidade, declaração incerta ou incompatibilidade conhecida.
- [ ] Evidência pertence ao dispositivo inspecionado e desaparece quando alvo muda ou dados deixam de ser válidos.
- [ ] Entrada manual permanece disponível quando ausência na declaração não prova incompatibilidade.
- [ ] Falha HAL/versionamento orienta retry/correção e não bloqueia futuras sessões por estado stale.
- [ ] Declaração de size/FPS ou fila de torch/zoom não é tratada como garantia física.

## Verification

Parser/helper cobrem target isolation; hardware e versão Android documentados validam o gate escolhido.
