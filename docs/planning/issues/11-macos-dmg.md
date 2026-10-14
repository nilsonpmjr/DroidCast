# 11: Instalar e executar pelo DMG no macOS

**ID:** N11

**What to build:** Abrir DMG, instalar e executar pelo Finder sem dependências do desenvolvedor nas arquiteturas/versões declaradas.

**Blocked by:** 10 / N10 — Produzir runtime independente do ambiente de desenvolvimento.

**Status:** ready-for-agent

**Implementação:** não iniciada. O rótulo de triagem não significa que os bloqueios foram concluídos.

**Origem / reconciliação:** L10; geração de .app atual não comprova standalone deployment.

## Acceptance criteria

- [ ] DMG inclui bundle completo com frameworks, plugins Qt e bibliotecas dos auxiliares.
- [ ] Candidatos Apple Silicon e Intel são verificados separadamente, condicionados ao toolchain/Qt escolhido; universal não é obrigatório.
- [ ] Assinatura e notarização contemplam componentes distribuídos; sem credenciais pode existir candidato de teste, não release pública qualificada.
- [ ] Teste em máquina limpa abre pelo Finder, descobre celular e executa mirror/capture/record/stop.
- [ ] Upgrade preserva preferências e mídia; mínimo de OS e arquitetura declarados correspondem à prova.
- [ ] Artifact hash/revisão e resultado de assinatura/notarização são registrados.

## Verification

Clean-machine por arquitetura declarada, fora do checkout e sem Homebrew/Qt SDK.
