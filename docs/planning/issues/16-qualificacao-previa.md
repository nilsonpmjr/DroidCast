# 16: Qualificar a prévia compacta multiplataforma

**ID:** N14

**What to build:** Produzir relatório de uma prévia com interface compacta e três candidatos de pacote, distinguindo comportamento validado de experimental.

**Blocked by:** 04 / N04 — Conexão Wi-Fi; 06 / N06 — Modos especialistas; 09 / N09 — Convenções nativas; 11 / N11 — DMG; 12 / N12 — EXE; 13 / N13 — AppImage; 14 / N17 — Oferta por conexão USB; 15 / N18 — Inicialização opt-in na sessão do usuário. N08 é dependência transitiva de N18.

**Status:** ready-for-agent

**Implementação:** não iniciada. O rótulo de triagem não significa que os bloqueios foram concluídos.

**Origem / reconciliação:** L01–L06/L08–L12/L14–L23/L25 e release-proof do gap register.

## Acceptance criteria

- [ ] Validar oferta USB sem duplicatas, envio explícito, monitoramento apenas com app em execução e inicialização opt-in/fallback nos pacotes aplicáveis.
- [ ] Testar USB/Wi-Fi, stop/playback de gravação, unplug/reconnect e retorno ao estado recuperável.
- [ ] Provar captura/APK/transferência, input/clipboard/drop e áudio/modos que forem anunciados como validados.
- [ ] Registrar bridge, remoto/túnel, FPS e branding nas plataformas aplicáveis; lacunas hardware permanecem explícitas.
- [ ] Artefatos vêm da mesma revisão candidata com hashes; relatório identifica OS, arquitetura, Android e condições de teste.
- [ ] Recurso sem prova pode ser declarado experimental/limitado, mas não encerra o critério completo antigo.
- [ ] Publicação externa é ação separada; gerar candidatos/relatório não publica uma release.

## Verification

Gate integrado com dispositivos reais e pacotes; helper não substitui essas provas.
