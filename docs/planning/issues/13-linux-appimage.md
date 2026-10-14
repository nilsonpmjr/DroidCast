# 13: Abrir AppImage em diferentes famílias Linux

**ID:** N13

**What to build:** Executar candidato AppImage x86_64 em distros representativas com contrato explícito de compatibilidade.

**Blocked by:** 10 / N10 — Produzir runtime independente do ambiente de desenvolvimento.

**Status:** ready-for-agent

**Implementação:** não iniciada. O rótulo de triagem não significa que os bloqueios foram concluídos.

**Origem / reconciliação:** L08; amplia alcance sem alegar todas as distros ou runtimes musl.

## Acceptance criteria

- [ ] Fixar base de build e glibc mínima, incluindo contrato de bibliotecas de host deliberadamente exigidas.
- [ ] Empacotar Qt/plugins, SDL, FFmpeg e libusb conforme contrato; não herdar dependência acidental da distro de build.
- [ ] Testar Debian/Ubuntu, Fedora e Arch em versões registradas, com X11/Wayland e desktop sem tray.
- [ ] Documentar requisitos de execução/montagem do AppImage e orientação de permissão USB sem alterar sistema silenciosamente.
- [ ] Ambiente limpo completa mirror/capture/record/stop; versões/arquiteturas não verificadas não são anunciadas como suportadas.
- [ ] Registrar hash/revisão e evidências de pacote.

## Verification

Jornadas em ambientes Linux representativos; inspecionar dependências e paths do artefato.
