# 21: Qualificar cobertura completa e reconciliar o registro final

**ID:** N16

**What to build:** Auditar opções/ações e evidências, preservando todas as pendências antigas até a qualificação do escopo completo.

**Blocked by:** 16 / N14 — Qualificar a prévia compacta multiplataforma; 17 / N15-A — Configurar um parâmetro codec avançado; 18 / N15-C — Explicar disponibilidade de configuração câmera; 19 / N15-O — Recuperar seleção OTG entre múltiplos dispositivos; 20 / N15-V — Consumir frames num dispositivo V4L2 existente.

**Status:** ready-for-agent

**Implementação:** não iniciada. O rótulo de triagem não significa que os bloqueios foram concluídos.

**Origem / reconciliação:** L26, demais L01–L25 e feature map.

## Acceptance criteria

- [ ] Relacionar cada opção/ação a destino, disponibilidade, persistência, launch/live/policy e prova.
- [ ] Revisar L01–L26 individualmente; redesign ou teste fake não encerra hardware/release sem evidência.
- [ ] Categorias são descobríveis sem sidebar permanente; nenhuma ação existente desaparece silenciosamente.
- [ ] Resolver explicitamente a prioridade de embedding e multi-file; exclusões não são descritas como implementações.
- [ ] Gerar tickets verticais adicionais para resíduos de auditoria; eles passam a bloquear este gate antes de alegar cobertura completa.
- [ ] Atualizar gap register e histórico por fatos, registrando suporte real e limitações dos artefatos.

## Verification

Auditoria cruzada, evidências de hardware/pacote e inventário sem itens não classificados.
