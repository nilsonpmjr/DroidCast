# 06: Iniciar modos especialistas com controles contextuais

**ID:** N06

**What to build:** Escolher câmera, somente áudio, display virtual ou controle USB/OTG e completar configuração, início e parada usando os caminhos existentes.

**Blocked by:** 05 / N05 — Configurar e lançar sem perder preferências avançadas.

**Status:** ready-for-agent

**Implementação:** não iniciada. O rótulo de triagem não significa que os bloqueios foram concluídos.

**Origem / reconciliação:** L18/L20–L22/L25; preserve implementação e pendências de hardware.

## Acceptance criteria

- [ ] Cada modo completa escolha → configuração → launch → estado respaldado pelo motor → stop.
- [ ] OTG tem entrada sem dispositivo ADB e picker libusb próprio; não herda discovery ADB como prova USB.
- [ ] Somente áudio oculta ferramentas de vídeo; câmera expõe torch/zoom conforme readiness/política e virtual display mantém app picker.
- [ ] Alternar modos conserva configurações inativas; read-only e restrições de alvo seguem snapshots.
- [ ] Resultados de fila e declarações de hardware não são apresentados como mudanças físicas garantidas.
- [ ] Se necessário, dividir a execução por modo sem mudar escopo ou bloqueios externos; cada divisão precisa entregar uma jornada completa.

## Verification

Helper cobre isolamento de modos e políticas; provas Android/câmera/áudio/USB continuam separadas no gate de release.
