# 14: Oferecer ações ao conectar Android via USB

**ID:** N17

**What to build:** Com DroidCast em execução, conectar um Android USB reconhecido oferece Espelhar, Transferir arquivo… ou Agora não, sem executar nenhuma operação antes da escolha.

**Blocked by:** 07 / N07 — Acesso pelo tray/menu bar opcional.

**Status:** in-progress

**Implementação:** oferta real no piloto compacto com assinatura persistente `adb track-devices -l`, substituindo polling após preferência explícita do usuário. Baseline, framing, retry, fila/alvo e parada testados; sonda validou stream ADB real no macOS. Fluxo físico/notificações/latência por plataforma ainda pendentes; não é observação USB universal nem login.

**Origem / reconciliação:** Novo gatilho para L01/L03/L05 e a transferência explícita já implementada; amplia N01/N07 sem implementar MTP ou acesso a pastas.

## Acceptance criteria

- [ ] Observação pertence ao aplicativo em execução; ocultar a janela não a interrompe se background foi habilitado; Sair encerra o observador/processos próprios e USB não reabre o app.
- [ ] Dispositivos já presentes no lançamento viram baseline, sem ofertas em massa; varreduras e unauthorized → device não geram novas perguntas para a mesma conexão.
- [ ] Oferta discreta não rouba foco; pode ser descartada ou desabilitada. Wi-Fi, emulador e discovery remoto não são interpretados como plug USB local.
- [ ] Identidade/autorização/estado são revalidados ao clicar; desconexão não redireciona alvo, sessão em andamento não é encerrada implicitamente.
- [ ] Espelhar exige pré-requisitos ADB; Transferir arquivo… reutiliza escolha explícita de arquivo/destino, cancelamento e resultado, sem enviar nada automaticamente.
- [ ] Android sem debugging pode não aparecer em ADB; registrar esse limite e avaliar observação USB nativa somente se necessária, sem tratar todo candidato libusb como Android autorizado.
- [ ] Sem tray, a oferta e os comandos permanecem acessíveis na janela; política efetiva de read-only segue o controlador.

## Verification

Helper cobre conexão nova, baseline, duplicatas, autorização, reconnect, desconexão antes do clique e target isolation. Telefone real comprova oferta, escolha e envio/espelhamento. Medir latência física; receber eventos ADB não prova detecção de todos os dispositivos USB.

As suítes do piloto passaram com esses casos e com reconexão do mesmo serial durante
a escolha de arquivo (sem envio). Resultado de scan foi adicionado ao controlador
sem mudar suas políticas: falha não significa ausência física. Wi-Fi/emulador/remoto
não oferecem ações USB. A implementação por eventos processa cada frame completo,
sem consulta periódica e sem colapsar as transições num refresh. Framing dividido/
agrupado, desconexão/retry, pausa OTG, isolamento de rota e saída também passaram.
Sonda de cinco segundos recebeu um único snapshot vazio do ADB real e encerrou o
cliente próprio. Ausência de telefone impede concluir a validação física deste ticket.
