# 05: Configurar e lançar sem perder preferências avançadas

**ID:** N05

**What to build:** Abrir configurações sob demanda, pesquisar opções comuns ou flags, editar, validar e iniciar usando preferências preservadas.

**Blocked by:** 03 / N03 — Espelhar e iniciar com gravação pela janela compacta.

**Status:** ready-for-agent

**Implementação:** não iniciada. O rótulo de triagem não significa que os bloqueios foram concluídos.

**Origem / reconciliação:** L06/L13/L17–L19/L25; preservar categorias e validação existentes; opções faltantes continuam nos resíduos.

## Acceptance criteria

- [ ] Jornada de resolução cobre pesquisa → edição → validação → persistência → argumento de lançamento.
- [ ] Configurações básicas e avançadas compartilham os mesmos valores; importação das preferências atuais não descarta dados.
- [ ] Valores inativos são preservados e omitidos quando inaplicáveis; mensagens distinguem próximo lançamento de ação ao vivo.
- [ ] Input, áudio, gravação e rotas avançadas existentes continuam descobríveis; campos inválidos indicam correção.
- [ ] Inventariar o destino de cada controle antes de remover a página antiga; ausência funcional não vira controle fictício.
- [ ] Presets permanecem opcionais; não são condição para concluir a migração.

## Verification

Round-trip por preferências/argumentos e jornada de UI; um lançamento real comprova o ajuste de resolução.
