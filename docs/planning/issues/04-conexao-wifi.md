# 04: Conectar por Wi-Fi sem perder rotas avançadas

**ID:** N04

**What to build:** Parear e conectar pelo fluxo guiado, selecionar o celular na janela compacta e preservar configurações de ADB remoto e túneis.

**Blocked by:** 03 / N03 — Espelhar e iniciar com gravação pela janela compacta.

**Status:** ready-for-agent

**Implementação:** não iniciada. O rótulo de triagem não significa que os bloqueios foram concluídos.

**Origem / reconciliação:** L04/L23 e checkpoint Connection and Tunnels; transportes já implementados são reutilizados.

## Acceptance criteria

- [ ] Portas de pareamento e conexão são diferenciadas e o fluxo termina num dispositivo selecionável.
- [ ] Código incorreto, endpoint inválido e timeout permitem correção/retry sem falso sucesso.
- [ ] Segredos de pareamento não vão a preferências nem logs; toda família de subprocessos usa a rota adequada.
- [ ] Remoto/túneis ficam em configuração avançada, com retorno ao local; sessão ativa preserva snapshot.
- [ ] Não criar SSH nem expor servidor ADB automaticamente.

## Verification

Helper cobre falhas/segredos/rotas; telefone real cobre pair-to-mirror e ambiente controlado cobre ADB remoto/SSH.
