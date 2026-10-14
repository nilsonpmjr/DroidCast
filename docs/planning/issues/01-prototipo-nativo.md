# 01: Comparar a experiência compacta em protótipo nativo

**ID:** N01

**What to build:** Experimentar uma janela compacta de dispositivos, menus, uma sessão contextual e acesso opcional pelo tray/menu bar antes de migrar o aplicativo existente.

**Blocked by:** None (can start immediately).

**Status:** in-progress

**Implementação:** protótipo Qt separado criado e testado no macOS. Após revisão, o usuário autorizou conectar a janela compacta ao motor real para um piloto de display. Prova SDL/foco, revisão interativa e comparação Windows/Linux ainda pendentes; não confundir o piloto N02/N03 com conclusão deste gate.

**Origem / reconciliação:** Novo; valida decisões visuais que substituem a navegação histórica de L11/L13.

## Acceptance criteria

- [x] Demonstrar oferta por conexão USB com Espelhar, Transferir arquivo… e Agora não; mostrar autorização pendente e desligamento da oferta, sem envio ou espelhamento automático.
- [x] Protótipo Qt separado do aplicativo existente demonstra selecionar dispositivo, escolher modo, acessar configurações e encerrar uma sessão; dados simulados são identificados.
- [ ] Comparar janela compacta com e sem tray/menu bar, incluindo cenário de tray indisponível e caminho de restauração.
- [ ] Executar uma prova com a janela SDL real em foco no macOS e registrar acesso aos comandos, restauração e limitações; simulação não vale como prova de integração.
- [ ] Registrar decisões de layout, nomes de menus, toolbar, tamanho mínimo e política opt-in de segundo plano; versões não verificadas permanecem explicitamente pendentes.
- [x] Preservar o motor e a interface atuais enquanto a proposta é avaliada; não exigir vídeo integrado para concluir esse experimento.

## Verification

Revisão visual e demonstração dos fluxos; prova nativa de foco/menus separada dos estados simulados.

Primeira entrega: [roteiro e decisões do protótipo](../../../desktop/prototype/README.md).
Build Debug macOS e suíte `prototype-tests` aprovadas; captura offscreen revisada.
Não há discovery USB real, transferência, configuração de login ou espelhamento nesse alvo.
Os cenários sem tray são cobertos automaticamente; o comportamento nativo ainda exige
revisão do desktop. Não encerrar o ticket com base somente nessa simulação.
