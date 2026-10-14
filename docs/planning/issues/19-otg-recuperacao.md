# 19: Recuperar seleção OTG entre múltiplos dispositivos

**ID:** N15-O

**What to build:** Selecionar e recuperar o alvo de controle USB quando candidatos somem, mudam ou são ambíguos, sem depender de ADB.

**Blocked by:** 06 / N06 — Iniciar modos especialistas com controles contextuais.

**Status:** ready-for-agent

**Implementação:** não iniciada. O rótulo de triagem não significa que os bloqueios foram concluídos.

**Origem / reconciliação:** L22 e Part 26; enumeration já existe, o novo trabalho é recovery.

## Acceptance criteria

- [ ] Picker usa relatório libusb existente; não inventa lista a partir de ADB.
- [ ] Ausência, serial duplicado/ambíguo e múltiplos candidatos produzem escolha/erro recuperável; não selecionar aparelho arbitrariamente.
- [ ] Refresh/reconnect mantêm ou invalidam seleção de forma explícita sem alterar alvo de sessão ativa.
- [ ] Falha de driver/acesso e unplug permitem retry, mantendo promessa de control-only.
- [ ] Linux/Windows hardware prova pelo menos seleção e recuperação sem USB debugging; plataformas não verificadas permanecem pendentes.

## Verification

Relatórios controlados cobrem ambiguity; dispositivos reais comprovam permissões, drivers e reconnect.
