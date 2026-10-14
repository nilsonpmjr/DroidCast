# 17: Configurar um parâmetro codec avançado

**ID:** N15-A

**What to build:** Editar uma opção de codec suportada, validá-la, preservar preferência e iniciar sessão usando o valor escolhido.

**Blocked by:** 05 / N05 — Configurar e lançar sem perder preferências avançadas.

**Status:** ready-for-agent

**Implementação:** não iniciada. O rótulo de triagem não significa que os bloqueios foram concluídos.

**Origem / reconciliação:** L17/L18; apenas lacuna comprovada, sem refazer encoders já implementados.

## Acceptance criteria

- [ ] Escolher um parâmetro documentado do motor e completar UI → validação → persistência → argumento → sessão.
- [ ] Valores viram argumentos estruturados, nunca comandos shell; erros e inaplicabilidade têm correção visível.
- [ ] Vídeo/áudio e encoder específico são distinguíveis; valor inativo não vaza para outro modo.
- [ ] Prova em telefone/encoder compatível demonstra efeito; rejeição do encoder não vira sucesso.
- [ ] Auditar constraints/alignment/apresentação remanescentes e criar tickets verticais próprios; este ticket não implementa todo esse grupo.

## Verification

Helper valida construção/isolation; encoder real valida uma configuração documentada.
