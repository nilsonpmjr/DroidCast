# 12: Instalar e executar pelo EXE no Windows

**ID:** N12

**What to build:** Instalar, executar, atualizar e desinstalar DroidCast por instalador EXE completo.

**Blocked by:** 10 / N10 — Produzir runtime independente do ambiente de desenvolvimento.

**Status:** ready-for-agent

**Implementação:** não iniciada. O rótulo de triagem não significa que os bloqueios foram concluídos.

**Origem / reconciliação:** L09; proposta aprovada de EXE via Inno Setup, MSI pode ser expansão.

## Acceptance criteria

- [ ] Instalador inclui runtime, DLLs e plugins necessários e não depende de SDKs/ADB externos.
- [ ] Caminhos com espaços/Unicode e lançamento normal não abrem consoles inesperados.
- [ ] Upgrade preserva preferências/mídia; uninstall não remove gravações do usuário.
- [ ] Windows mínimo/arquitetura e status de assinatura são declarados com evidência.
- [ ] Máquina limpa completa USB mirror/capture/record/stop e mostra orientação para falhas de driver/autorização.
- [ ] Registrar hash e revisão do candidato.

## Verification

Install/upgrade/uninstall e jornada de telefone real em ambiente Windows limpo.
