# Protótipo da interface compacta — N01

Experimento Qt Widgets separado do aplicativo principal. Usa aparência e menus do
sistema. Agora abre por padrão a **interface compacta experimental com motor real**:
descoberta ADB, espelhamento, gravação no lançamento, captura, transferência explícita,
ajuste da janela e stop. Tray/background opcionais agora têm preferências próprias
salvas; configurações de mídia do workspace principal não são alteradas e iniciar
com o sistema ainda não é registrado.

O modo `--demo` mantém a simulação isolada: sem processos Android, arquivos enviados
ou vídeo real. Os modos são diferentes e identificados na janela. Apenas display
está conectado ao motor neste piloto; o aplicativo principal continua disponível
com suas configurações e modos especialistas, ainda não migrados para essa interface.

## Compilar e executar

Com Qt 6.4+ Widgets/Test disponível no prefixo de desenvolvimento:

```sh
cmake -S desktop/prototype -B build-prototype -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-prototype --parallel 2
ctest --test-dir build-prototype --output-on-failure
```

No macOS, abra `build-prototype/droidcast-prototype.app`. Em Windows/Linux,
execute `droidcast-prototype` na pasta de build (com `.exe` no Windows).
Se Qt não estiver no prefixo padrão, configure `CMAKE_PREFIX_PATH` com seu kit.
Esta build é de desenvolvimento, não um pacote distribuível independente.

O build do piloto compila o controlador existente. Por padrão, copia o runtime de
desenvolvimento `build-droidcast/runtime` ao lado do executável, dentro do `.app`
no macOS. Para outro build, passe `-DDROIDCAST_PROTOTYPE_RUNTIME_SOURCE=<diretório>`.
Precisa do client **fork**, ADB e server compatível; não selecionar o client upstream
sem lifecycle/bridge. Bibliotecas nativas ainda dependem do toolchain local existente;
N10 permanece responsável por deployment independente.

## Usar espelhamento real

1. Abra o app sem `--demo`. A janela diz Espelhamento real; nenhuma sessão começa sozinha.
2. Conecte Android com depuração USB habilitada e aceite a autorização no aparelho.
3. Selecione o serial correto; a descoberta acompanha eventos ADB enquanto o app
   está aberto. Atualizar dispositivos reinicia explicitamente a assinatura.
4. Clique Espelhar. O vídeo abre na janela SDL real; somente o primeiro frame muda
   a interface para Espelhando. Observe falhas em Ajuda → Diagnóstico.
5. Exibir → Ajustar janela e seu botão enviam o mesmo comando live pelo controlador.
6. Sessão → Iniciar com gravação… escolhe MKV antes do lançamento; não inicia gravação
   no meio de uma sessão existente. Capturar imagem… salva PNG no alvo explícito.
7. Dispositivo → Transferir arquivo… escolhe arquivo e diretório Android e envia
   realmente pelo fluxo ADB existente. Aviso de substituição aparece no diálogo;
   Cancelar transferência usa o cancelamento do controlador.
8. Encerrar sessão e Sair usam parada graciosa; sair durante transferência solicita
   cancelamento e aguarda operação/processo terminar antes de sair. Fechar a janela
   também encerra operações, a menos que background tenha sido habilitado com tray disponível.

Runtime usa defaults bundled de display. Configurações completas de mídia, modos
especialistas e login ainda não estão implementados no **modo real**. A primeira
oferta USB real agora usa a assinatura de eventos descrita abaixo; não detecta Android
que não esteja reconhecido pelo ADB.
Não executar testes fake como prova de espelhamento no telefone.

## Tray/menu bar do modo real

1. Em Configurações, habilite Mostrar ícone no tray / barra de menus.
2. Opcionalmente habilite Manter em segundo plano ao fechar a janela. Ambas estão
   desligadas inicialmente e persistem separadamente em `DroidCast/CompactPilot`.
3. Com sessão ativa, feche a janela Qt: com essa política e tray disponível, a
   sessão continua. O ícone oferece Abrir DroidCast, captura, transferência,
   comandos de imagem e Encerrar sessão, compartilhados com a barra de menus.
4. Use Imagem → Ajustar janela/Tela cheia/Rotação/Pausar/Retomar para agir sobre
   o renderer real, sem sintetizar teclas nem depender de foco Qt.
5. Encerrar sessão pelo tray para o motor e mantém o aplicativo disponível.
   Sair sempre finaliza/cancela operações e termina o aplicativo, mesmo com background.
6. Abrir DroidCast restaura a janela Qt; não promete trazer a janela SDL para frente.
7. Se o desktop deixar de oferecer tray (verificação periódica da disponibilidade
   Qt), a janela oculta é restaurada. Sem tray disponível, fechar nunca oculta a
   única janela recuperável e usa o encerramento normal.

Pausar imagem não pausa Android, áudio nem gravação. Autostart N18 ainda não é
registrado. A primeira oferta N17 é descrita abaixo. Ícone e restauração em cada desktop precisam
de prova real; os testes injetam apenas a disponibilidade para reproduzir sua perda.

## Oferta ao conectar USB — N17 por eventos

- Mantenha o app aberto (ou oculto com background/tray habilitados), deixe a consulta
  inicial terminar e conecte um Android que o ADB reconheça. A janela mostra o alvo
  e Espelhar / Transferir arquivo… / Agora não, sem executar operações automaticamente.
- Enquanto oculta, usa notificação onde o sistema suporta e um submenu USB vinculado
  ao serial no tray. Clique de notificação apenas restaura a janela, sem iniciar sessão.
  Sem notificações, o submenu/tooltip e a janela continuam oferecendo acesso.
- Nenhuma seleção é alterada ao chegar um dispositivo. Escolher uma ação da oferta
  seleciona explicitamente o alvo anunciado e revalida autorização/disponibilidade.
- Durante outra sessão/operação, a oferta fica indisponível e não interrompe o trabalho.
  Mais de uma chegada é enfileirada; descartar uma oferece a próxima.
- Autorizar o mesmo Android atualiza a oferta existente; varreduras repetidas não
  repetem a pergunta. Dispositivos já presentes ao lançar o app são baseline silencioso.
- Agora não descarta aquela chegada. Desconectar/reconectar pode oferecer novamente.
  Configurações permite desligar ofertas; essa escolha persiste no namespace do piloto.
- Falha/timeout de descoberta não representa unplug: preservar baseline evita perguntas
  falsas quando a consulta se recupera. Wi-Fi, emulador e discovery remoto não são ofertas USB locais.
- Desconexão observada antes do clique invalida a oferta. Durante escolha de arquivo,
  reconexão observada do mesmo serial também invalida aquela tentativa de transferência.
- Sair termina o monitoramento do app; não registrar serviço nem login. ADB compartilhado
  não é encerrado arbitrariamente. Nenhuma transferência/pasta/MTP acontece ao conectar.

Detecção atual: um único cliente persistente `adb track-devices -l`, com stdout
interpretado como mensagens de comprimento hexadecimal de quatro bytes. Mensagens
fragmentadas ou agrupadas são processadas por snapshot, sem perder transições ao
coalescer tudo numa consulta. Nenhum timer dispara `adb devices` no piloto compacto.
O workspace principal agora compartilha a mesma assinatura persistente e removeu
as consultas periódicas de descoberta. A oferta USB permanece exclusiva do piloto.

Falha da assinatura invalida alvos/ofertas e estabelece nova referência após recuperar,
sem inventar unplug. Repetição acontece apenas em falhas, com espera progressiva de
1 a 30 segundos; estabilidade permite voltar à espera inicial. Não há timeout de
inatividade após a primeira mensagem válida, mesmo com nenhum telefone conectado.
O botão Atualizar reinicia a assinatura, mantendo a seleção quando possível.

Sair para o cliente próprio e cancela retry; não encerra ADB compartilhado. A assinatura
pausa durante OTG e retoma depois; remoto respeita o snapshot de transporte da sessão.
Inicialização automática continua fora desta entrega. O timer de disponibilidade do
tray permanece separado e não executa consultas ADB.

Limites: Android sem depuração USB pode não aparecer; a assinatura não observa todos
os USB do sistema. Se o servidor/observador estiver indisponível, eventos naquele intervalo
não são reconstruídos como conexões físicas. Latência física, notificações, reconexão
e fluxo por plataforma ainda precisam de validação; N17 continua parcial por essas provas.

Fonte de protocolo: [serviços do ADB](https://android.googlesource.com/platform/packages/modules/adb/+/7799b4ba18dd0fc0298ccac65014ca533dacbddd/SERVICES.TXT)
e [CLI que encaminha track-devices](https://android.googlesource.com/platform/packages/modules/adb/+/refs/heads/main/client/commandline.cpp).

## Roteiro de revisão

Este roteiro é do modo simulado: execute com `--demo`.

1. Abra com nenhum dispositivo. Selecione um dos dois alvos simulados.
2. No menu Demonstração, conecte USB sem autorização. A oferta aparece, mas espelhar
   e transferir permanecem indisponíveis; o seletor mantém acesso ao modo OTG.
3. Autorize pelo menu. A oferta existente habilita as ações, sem outra pergunta.
4. Clique Agora não e repita descoberta: a oferta não volta. Desconecte/reconecte:
   a oferta reaparece, exceto se a preferência de supressão estiver marcada.
5. Escolha Espelhar: somente o estado simulado muda. Encerrar sessão está disponível
   pelo botão, menu e tray quando habilitado. Nenhuma janela de vídeo real é criada.
6. Transferir arquivo… abre um seletor e uma prévia de alvo/destino; fechar cancela.
   Não existe botão que envie bytes. Essa prévia ilustra o fluxo explícito existente.
7. Alterne modos antes de iniciar. Durante a sessão, dispositivo e modo ficam fixos.
8. Habilite Usar tray / menu bar no menu Demonstração, se o desktop oferecer suporte.
   Configurações permite fechar para tray; restaurar vem do menu/ícone do tray.
   Conexão simulada com janela oculta oferece notificação, onde suportada.
9. Simule tray indisponível: a janela é restaurada e fechar não a oculta.
10. Sair encerra o protótipo. Plugar um Android não inicia nada; não há observador real.

CLI para reprodução visual da simulação: `--demo`, `--connected`, `--unauthorized`, `--session`, `--no-tray`
e `--screenshot <arquivo.png>`. Screenshot captura apenas a janela Qt; offscreen não
comprova menu global macOS nem interação do tray.

## Decisões em experimentação

- Janela inicial 640×460, mínimo 540×400, sem sidebar permanente.
- Seleção e modo no centro; uma oferta contextual por conexão; estado e encerramento
  abaixo. Oferta inline é um experimento: não substituir automaticamente por popup invasivo.
- Dispositivo/Sessão/Ferramentas/Ajuda contêm ações; Demonstração é exclusivamente
  instrumentação do protótipo e não faz parte do produto final.
- Papéis nativos Qt para Configurações, Sobre e Sair; preferência visual do sistema.
- Background e tray opt-in, sem persistência nesta demonstração. Login permanece
  identificado como planejado; mecanismos reais pertencem a N18.
- Transferência durante sessão está conservadoramente indisponível neste experimento;
  migração N02/N03 deve preservar o contrato real por modo, sem copiar esse limite mock.
- O fluxo de somente leitura e demais capacidades do controlador não é implementado
  neste alvo; sua migração/teste pertence aos tickets de produção.

## Evidência e pendências

Build e testes de comportamento simulados foram executados no macOS. Captura
offscreen revisada para cenário conectado; não prova aparência Cocoa/menu global.
Os testes cobrem não auto-iniciar, descarte sem duplicatas, autorização/unplug,
supressão, alvo invalidado e ações de menu/botão sem tray.

N01 continua **em andamento**: validar foco/acesso aos menus com a janela SDL real
em outro processo, restauração nativa, notificação/tray, teclado e comparação real
Windows/Linux. Não criar uma segunda QWidget e chamá-la de prova SDL.

Para a prova SDL: abrir protótipo e a janela real do motor em ambiente autorizado,
alternar foco, anotar quais menus globais pertencem ao processo ativo e se o tray
recupera acesso às ações. A prova precisa de observação do desktop; uma captura
offscreen não encerra esse critério. Sem telefone/ambiente disponível, manter pendente.

## Evidência do piloto com motor

As suítes `prototype-tests` e `compact-live-tests` passaram no macOS. O helper de
processos existente comprova serial fixado, rejeição de dispositivo unauthorized,
espera de primeiro frame, disponibilidade do comando Ajustar por menu/botão,
falha recuperável e fechamento aguardando o quit. Não prova frames reais.

Client staged executa `--version`. ADB bundled executou discovery após liberar o
socket local impedido pelo sandbox; nenhum telefone estava conectado nessa consulta.
Essa execução automatizada não comprovou espelhamento, transferência/captura ou playback
físico; a confirmação posterior de espelhamento pelo usuário está registrada abaixo.
O piloto foi autorizado pelo usuário para tornar a revisão real antes de concluir
toda a validação N01; não é a substituição final do workspace nem conclusão N02/N03.

Atualização de evidência: após testar essa build, o usuário informou “funcionou
conforme esperado” para o fluxo de espelhamento real. Essa confirmação vale para a
jornada relatada neste macOS, não comprova recording/playback, transferência, comandos
de imagem ou outras plataformas. Essas provas permanecem pendentes individualmente.

Tray/background: adicionados testes de fechar para background mantendo a sessão,
restore pelo menu, stop sem quit, indisponibilidade restaurando a janela, persistência
isolada e quit aguardando a saída apesar da preferência de background. Menus do tray
reutilizam as QActions do modo real. Revisão nativa com SDL em foco ainda pendente.

Oferta USB: as suítes passaram com baseline, chegada, autorização, descarte/reconnect,
falha de scan, Wi-Fi/emulador/remoto, alvo diferente da seleção, sessão ativa, várias
chegadas, supressão persistida e quit. Um teste com seletor de arquivo aceito cobre
unplug/reconnect do mesmo serial sem envio. O helper também verifica o novo resultado
de scan do controlador; um scan falho nunca é convertido em confirmação de unplug.

Eventos: testes com processo helper cobrem framing dividido/agrupado, zero dispositivos,
prefixo inválido, EOF truncado, falha/reconexão, rota herdada/ativa/remota, pausa OTG,
encerramento aguardando observador e ausência de novos processos/consultas em repouso.
As duas suítes do piloto e os testes de regressão selecionados do workspace passaram.

Sonda manual `compact-monitor-probe` (não registrada no CTest) executou o cliente real
no macOS durante cinco segundos. Recebeu um snapshot vazio em 37 ms e permaneceu com
uma única mensagem até encerrar seu cliente. Sem telefone conectado: isso prova
compatibilidade com o stream real, não latência de plug físico.

Medição curta com `/usr/bin/time -l`: 5,07 s decorridos, 0,07 s de CPU user + 0,03 s
system e máximo RSS reportado de 16 MiB. Inclui inicialização da sonda e do cliente;
não representa o app gráfico completo, o ADB compartilhado ou energia em repouso
prolongado. Não usar esses números como promessa de consumo para Windows/Linux.

Reprodução com runtime de desenvolvimento explicitamente indicado:

```sh
DROIDCAST_RUNTIME_DIR="$PWD/build-prototype/droidcast-prototype.app/Contents/MacOS/runtime" build-prototype/compact-monitor-probe
```

Em Windows/Linux, use o runtime correspondente. O probe só lê a lista e termina
seu próprio observador; não espelha nem envia arquivos.
