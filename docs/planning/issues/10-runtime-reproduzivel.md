# 10: Produzir runtime independente do ambiente de desenvolvimento

**ID:** N10

**What to build:** Gerar um candidato completo que inicia e descobre celulares fora do checkout, com componentes compatíveis e origem verificável.

**Blocked by:** None (can start immediately).

**Status:** ready-for-agent

**Implementação:** não iniciada. O rótulo de triagem não significa que os bloqueios foram concluídos.

**Origem / reconciliação:** L07 e Distribution; elimina dependências privadas/temporárias da build relocada.

## Acceptance criteria

- [ ] Dependências fixadas, hashes, revisão, manifesto e avisos/licenças acompanham o candidato.
- [ ] Client fork, ADB e server têm origem/versionamento compatíveis; divergência do server upstream exige construir o correspondente.
- [ ] Qt, plugins e bibliotecas do motor/subprocessos são resolvidos pelo pacote, sem prefixos privados ou toolchain em /private/tmp.
- [ ] Build limpa documentada gera o candidato a partir dos inputs fixados; caches são reutilização e não condição implícita.
- [ ] Executar fora do checkout e sem ferramentas Android/SDKs no PATH não exige localizar executáveis.
- [ ] Uso USB funciona sem downloads após instalação.

## Verification

Build limpa, manifesto, inspeção de dependências e launch/discovery em ambiente separado.
