# DroidCast: interface compacta e distribuição multiplataforma

Status: planejamento aprovado pelo usuário; registrado localmente em `docs/planning`.
Data: 2026-10-13. Preparada com `to-spec` a partir da conversa e do código atual.
Confirmação: “pode prosseguir assim”, seguida de autorização para criar os documentos.
Execução e provas de hardware/plataforma ainda não são consequência desta aprovação.

## Problem Statement

O usuário quer preservar o canivete suíço do scrcpy e distribuir DroidCast pronto
para uso em Windows, macOS e várias distribuições Linux. A interface atual organiza
esse poder em sete destinos e numerosos cartões e controles permanentes. Isso torna
uma tarefa cotidiana — escolher um celular e espelhar — mais complexa que o necessário.

O produto também tem desenvolvimento de funcionalidades, reorganização visual e
validação de distribuição misturados no planejamento. Uma reorganização não deve
reimplementar o que funciona nem apagar funcionalidades e provas ainda pendentes.
Compilar na máquina de desenvolvimento não significa produzir um pacote independente.

## Solution

Propor um utilitário compacto com seleção de dispositivos, início explícito de sessão,
menus completos e controles contextuais. Preservar configurações avançadas, descoberta
de capacidades e modos especialistas. Adaptar apresentação e convenções às plataformas,
com atenção especial a uma experiência macOS limpa e familiar.

O protótipo deve comparar janela compacta com menus e essa mesma janela acompanhada
de acesso rápido pelo tray/menu bar. Tray é complemento opcional; a janela permanece
suficiente em ambientes sem essa integração. O plano usa background opt-in;
detalhes de interação e restauração são validados pelo protótipo.

Com o aplicativo em execução, conectar um Android via USB oferece “Espelhar”,
“Transferir arquivo…” e “Agora não”. A transferência reutiliza o envio explícito
existente, com arquivo e destino escolhidos pelo usuário; não abre armazenamento MTP
nem adiciona um navegador de arquivos. Iniciar com o sistema é uma preferência opt-in.

Preparar distribuição completa, com DMG no macOS, proposta de instalador EXE no Windows
e proposta de AppImage no Linux. Formatos, arquiteturas e versões mínimas devem ser
confirmados por provas de pacote, sem prometer todas as distribuições ou versões.

## User Stories

1. Como usuário, quero escolher um celular e espelhar com poucas decisões, para começar rapidamente.
2. Como iniciante, quero instruções de conexão quando necessárias, para não depender do terminal.
3. Como usuário, quero distinguir autorização pendente, offline e acesso USB negado, para corrigir o problema.
4. Como usuário com vários dispositivos, quero identificar o alvo de cada ação, para evitar operações no celular errado.
5. Como usuário, quero iniciar uma sessão explicitamente, para controlar quando minha tela aparece.
6. Como usuário, quero conectar por Wi-Fi em um fluxo guiado, para entender pareamento e conexão.
7. Como usuário avançado, quero usar meu endpoint ADB e túnel existente, para preservar minha infraestrutura.
8. Como usuário, quero menus organizados por tarefa, para localizar funcionalidades sem uma tela carregada.
9. Como usuário frequente, quero atalhos visíveis nos menus, para executar ações rapidamente.
10. Como usuário macOS, quero menus e preferências nas posições habituais, para usar convenções familiares.
11. Como usuário Windows ou Linux, quero comandos equivalentes, para ter a mesma capacidade funcional.
12. Como usuário, quero poucos controles frequentes na sessão, para reservar espaço e atenção ao conteúdo.
13. Como usuário, quero todas as ações existentes acessíveis, para não perder recursos com a simplificação.
14. Como usuário, quero pesquisar opções por termos comuns e flags, para aproveitar a documentação scrcpy.
15. Como usuário, quero identificar configurações de próxima sessão, para não esperar mudanças ao vivo inexistentes.
16. Como usuário avançado, quero preservar preferências e valores inativos, para alternar modos sem perder ajustes.
17. Como usuário, quero alternar entre display, câmera, áudio, display virtual e OTG, para usar o modo adequado.
18. Como usuário de câmera, quero ver controles próprios desse modo, para operar sem ferramentas irrelevantes.
19. Como usuário OTG, quero selecionar USB sem autorização ADB, para controlar acessórios compatíveis.
20. Como usuário somente áudio, quero estado de áudio e gravação, para não confundir a ausência de vídeo com falha.
21. Como usuário em modo somente leitura, quero bloqueios iguais em menus, toolbar e tray, para preservar a política da sessão.
22. Como usuário, quero separar rotação Android de rotação da imagem, para alterar o destino correto.
23. Como usuário, quero iniciar com gravação usando formato compatível, para produzir arquivos utilizáveis.
24. Como usuário gravando, quero finalizar ao encerrar e saber quando houve interrupção, para interpretar o resultado corretamente.
25. Como usuário, quero abrir capturas e gravações reais, para conservar a biblioteca já disponível.
26. Como usuário, quero ocultar a janela de controle sem encerrar acidentalmente a sessão, se escolher trabalhar em segundo plano.
27. Como usuário, quero diferenciar fechar janela, encerrar sessão e sair, para entender o ciclo de vida do app.
28. Como usuário, quero restaurar o aplicativo facilmente, para não perder acesso aos controles.
29. Como usuário num desktop sem tray, quero todas as tarefas disponíveis pela janela, para usar o produto normalmente.
30. Como usuário, quero desligamento previsível durante transferências, para não abandonar operações silenciosamente.
31. Como usuário de teclado ou tecnologia assistiva, quero foco, nomes e estados acessíveis, para operar os mesmos recursos.
32. Como usuário, quero aparência legível em tema claro/escuro e escala elevada, para respeitar meu ambiente.
33. Como usuário, quero diagnósticos acessíveis quando há falhas, para resolver problemas sem exibir infraestrutura permanentemente.
34. Como usuário macOS, quero instalar por DMG e abrir pelo Finder, para dispensar Homebrew ou terminal.
35. Como usuário Windows, quero um instalador pronto e atualização que preserve meus dados, para dispensar SDKs e compiladores.
36. Como usuário Linux, quero um pacote utilizável em várias famílias de distros, para dispensar a montagem manual das dependências.
37. Como usuário USB, quero funcionar sem downloads após a instalação, para usar o app offline.
38. Como mantenedor, quero componentes compatíveis e builds reproduzíveis, para verificar a origem do pacote.
39. Como usuário, quero limitações por plataforma e hardware explícitas, para distinguir indisponibilidade de falha.
40. Como usuário avançado, quero que os recursos pendentes continuem no backlog, para que uma interface menor não reduza a ambição do produto.
41. Como usuário com DroidCast aberto, quero uma oferta ao conectar um Android USB, para escolher espelhamento ou transferência explícita.
42. Como usuário, quero escolher arquivo e destino antes de transferir, para controlar os dados enviados.
43. Como usuário, quero habilitar ou desabilitar iniciar com o sistema, para decidir quando DroidCast fica disponível.
44. Como usuário, quero que Sair encerre o monitoramento, para não manter um agente oculto após fechar o aplicativo.
45. Como usuário, quero uma oferta por conexão e feedback de autorização, para não receber perguntas repetidas durante a descoberta.

## Implementation Decisions

### Confirmado pelo usuário

- Preservar a abrangência funcional do scrcpy; simplificar a apresentação.
- Planejar Windows, macOS e ampla cobertura Linux; DMG no macOS e EXE ou MSI no Windows.
- Avaliar menus com funções principais e tray; valorizar uma apresentação limpa no macOS.
- Cruzar toda a esteira nova com os pendentes do planejamento existente.
- Monitorar conexões somente enquanto DroidCast estiver em execução, inclusive em segundo plano escolhido pelo usuário.
- Oferecer inicialização com o sistema opt-in e transferência explícita existente, sem acesso MTP/navegação de armazenamento.
- Esta etapa entrega planejamento; não autoriza automaticamente publicação de release.

### Propostas a validar no protótipo

- Manter Qt/C++ e o motor C atual. Uma troca de framework não é necessária para experimentar a nova organização.
- Substituir a navegação permanente de sete destinos por uma janela de dispositivos,
  diálogos de conexão/configuração e um estado compacto de sessão.
- Preservar a biblioteca de mídia como janela sob demanda; não remover seus filtros
  nem a proteção de arquivos ativos apenas para reduzir a navegação.
- Agrupar comandos em Dispositivo, Sessão, Exibir, Ferramentas e Ajuda; respeitar
  menu do aplicativo, Configurações e Sair no macOS. Validar nomes e quantidade visualmente.
- Manter comandos frequentes visíveis e levar demais ações aos menus. Toda ação com
  alvo deve deixar claro se usa dispositivo selecionado ou sessão ativa.
- Começar com display, câmera, somente áudio, display virtual e OTG como modos.
  Incluir uma entrada OTG própria mesmo sem dispositivo ADB. Conexão remota é uma rota,
  não um novo modo de mídia. V4L2 é saída Linux, não dispositivo de conexão.
- Configurações comuns e avançadas compartilham validação e persistência atuais.
  Presets só entram se demonstrarem utilidade no protótipo e tiverem composição documentada;
  não são dependência do redesenho.
- Tray/menu bar opcional oferece estado, mostrar janela, ações rápidas válidas e saída.
  Sua ausência nunca impede uso nem deixa o app inacessível.
- Lançamento manual abre a janela. Inicialização na sessão do usuário é uma preferência
  opt-in; pode abrir em segundo plano quando houver acesso comprovado de restauração.
  Sem esse acesso, manter a janela recuperável. Nenhum lançamento espelha automaticamente.
  Permanência após fechar também será opt-in.
- Manter espelhamento em janela separada na experiência inicial proposta. A decisão
  sobre vídeo integrado permanece aberta e precisa de experimento com frames reais
  antes de se tornar requisito. Aprovar o redesign não aprova embedding por consequência.

### Limites de comportamento

- Monitoramento pertence ao ciclo de vida do aplicativo, não ao da janela principal:
  ocultar com background habilitado mantém a observação; Sair encerra observadores
  e subprocessos próprios. Sem app em execução, plugar USB não inicia DroidCast.
- Oferecer ação uma vez por conexão física reconhecida, sem confundir varreduras,
  mudanças unauthorized → device, Wi-Fi, emulator ou dispositivos remotos com novo plug.
  Ao lançar o app, dispositivos já presentes aparecem na lista sem chuva de ofertas.
- A oferta não rouba foco nem cria sessão; respeita dismiss/supressão de ofertas.
  Verificar existência, identidade, estado de autorização e disponibilidade novamente
  ao clicar; unplug ou outra sessão ativa não redireciona o pedido nem a interrompe.
- Reutilizar discovery ADB local para dispositivos reconhecidos; Android sem debugging
  pode não aparecer. Avaliar eventos USB nativos se necessário, sem confundir candidato
  libusb com Android autorizado nem prometer detecção universal na primeira implementação.
- Após revisão do consumo, o usuário preferiu eventos. N17 usa assinatura persistente
  `adb track-devices -l`, sem consultas periódicas no piloto compacto. Processar cada
  snapshot/framing, recuperar falhas com backoff, invalidar histórico desconhecido e
  parar o cliente ao Sair. Isso não comprova hotplug universal: medir latência física
  e preservar o limite de reconhecimento/autorização do ADB.
- Espelhamento e envio explícito exigem seus pré-requisitos ADB. Oferecer autorização/
  debugging quando necessário, sem considerar que conexão USB já concede acesso.
- “Transferir arquivo…” abre o fluxo atual de escolha do arquivo e diretório Android,
  confirma o alvo e respeita cancelamento/resultado/read-only efetivo do controlador.
  Nenhum arquivo é enviado ao detectar conexão, nenhum filesystem é montado.
- Inicialização com o sistema usa mecanismo da sessão do usuário, reversível e
  desabilitado inicialmente. Sem serviço privilegiado, daemon separado ou observador
  persistente depois de Sair. Sair não remove a preferência de login, mas impede
  reativação automática por plug durante a sessão atual.
- Introduzir ou consolidar ações compartilhadas na fronteira da interface com o
  controlador existente. Menus, botões, atalhos e tray consomem disponibilidade e
  execução iguais; a validação definitiva continua no controlador.
- Reutilizar snapshots de política, modo, alvo e transporte da sessão ativa.
  Uma configuração para a próxima execução não enfraquece essas restrições.
- Preservar as diferenças entre consulta, comando ao vivo e opção de lançamento.
- O motor atual inicia gravação no lançamento. Usar “Iniciar com gravação…” antes
  da sessão; durante uma sessão sem gravação, não oferecer início ao vivo como recurso
  existente. Interromper/reiniciar exige escolha explícita, nunca perda silenciosa da sessão.
- “Pausar imagem” não pausa Android, áudio ou gravação. Confirmação de enfileiramento
  não se converte em confirmação de mudança física na câmera ou no Android.
- Fechar a janela de controle, fechar a janela SDL, encerrar a sessão e sair do app
  são eventos distintos. Validar cada um no protótipo e implementar política explícita.
- Sair durante gravação usa finalização existente e feedback de resultado; durante
  transferência resolve conclusão ou cancelamento explicitamente. Fallback forçado
  não apresenta arquivo como finalizado com sucesso.
- A janela SDL é outro processo: menus globais macOS e restauração/foco precisam de
  prova real. Não assumir que uma QAction da janela Qt acompanha o foco SDL.

### Distribuição proposta

- Pipeline reproduzível com dependências fixadas, manifesto, revisão de origem,
  checksums, componentes client/server compatíveis e licenças/avisos distribuídos.
- Remover dependência de caches relocados, diretórios temporários e prefixos privados
  da máquina do desenvolvedor nos artefatos finais.
- macOS: DMGs separados Apple Silicon e Intel inicialmente, condicionados ao suporte
  do Qt e do toolchain. Universal é opcional, não um requisito antecipado. Incluir
  bibliotecas dos processos auxiliares; assinar e notarizar o produto distribuído.
- Windows: propor EXE via Inno Setup; MSI somente por necessidade concreta. Definir
  versões/arquiteturas testadas; não confundir o executável do app com um instalador completo.
- Linux: propor AppImage x86_64, compilado na base mais antiga escolhida e validada.
  Registrar glibc mínima, Qt, SDL, FFmpeg, plugins e dependências de host deliberadas.
  Testar famílias Debian/Ubuntu, Fedora e Arch, desktops com/sem tray, X11 e Wayland.
- ARM Linux/Windows, Flatpak e pacotes específicos de distro são expansões a avaliar.
  Não prometer todas as distribuições, Alpine/musl ou versões obsoletas sem builds e provas próprias.
- Criar candidatos testáveis por plataforma em paralelo; publicação pública é uma etapa separada.
- Prévia pode conter lacunas declaradas; release com alegação de cobertura completa
  permanece condicionado aos itens funcionais e de hardware herdados.

## Testing Decisions

Fronteira proposta para revisão: jornadas visíveis acionam o controlador público já
existente. O helper de processos e Qt Test continuam fornecendo falhas determinísticas;
não é necessário introduzir novas APIs de teste no motor para redesenhar a interface.

- Demonstrar selecionar → configurar → lançar → agir → parar → recuperar pela interface.
- Testar equivalência de comandos por menu, toolbar e tray por efeito, alvo e estado;
  não exigir contagem de widgets, estrutura da sidebar ou pixels como contrato principal.
- Incluir transições de disponibilidade, somente leitura, modos incompatíveis,
  capacidades atrasadas e mudança de seleção durante uma operação.
- Verificar preservação de preferências, categorias e funcionalidades já existentes.
- Cobrir a ausência de tray e a política de fechar/sair em comportamento observável.
- Verificar baseline de dispositivos já presentes, evento duplicado, autorização,
  unplug antes do clique, conexão remota/Wi-Fi e saída interrompendo observadores.
- Validar habilitar/desabilitar login nos pacotes e fallback sem tray; transferência
  exige escolha explícita e mantém seu alvo apesar de mudanças de seleção.
- Provas reais separadas validam menu macOS com foco SDL, restauração de janelas,
  teclas da plataforma, accessibility, tema, escala e gravação finalizada.
- Pacotes são testados fora do checkout, sem SDKs/compiladores nem ferramentas Android
  preinstaladas, com caminhos Unicode/espaços e ambiente limpo.
- Reutilizar os casos de hardware pendentes: unplug/reconnect, periféricos, câmera,
  displays, áudio, clipboard, transferências, ADB remoto, OTG e V4L2 onde disponível.
- Evidência de pacote e funcionalidade fica vinculada a OS/arquitetura, hash e revisão.
  Resultado de helper não substitui validação do telefone ou do desktop real.

## Out of Scope

- Apagar opções avançadas, biblioteca de mídia ou modos especialistas para simplificar.
- Reescrever a aplicação em outro framework nesta fase.
- Implementar início/pausa de gravação ao vivo sem um requisito e integração próprios.
- Tornar tray obrigatório, iniciar no login sem opt-in ou espelhar/reconectar automaticamente.
- Abrir pastas Android/MTP, navegador de arquivos ou ativação pelo USB com app encerrado.
- Sessões concorrentes, perfis por dispositivo e automação de atualizações nesta fase.
- Instalar drivers/kernel modules silenciosamente, gerenciar credenciais SSH ou expor ADB automaticamente.
- Prometer integração do vídeo ou compatibilidade universal com distros/versões sem prova.

## Further Notes

Esta proposta revisa a organização visual antiga; o alcance funcional permanece.
As referências cruzadas e pendências preservadas estão na esteira reconciliada companheira.
Os documentos antigos permanecem como histórico e critérios de funcionalidades.
A nova direção está aprovada; layout final, mínimos de plataforma e detalhes de
integração são definidos pelos tickets correspondentes e pelas provas do protótipo.

Ferramenta de protótipo recomendada: Qt Widgets em alvo isolado e descartável, com
estados simulados explícitos e uma prova separada usando a janela SDL real. Isso permite
avaliar menus nativos, tray e foco. HTML/CSS local pode acelerar uma comparação visual,
mas não prova integração do sistema. Não há necessidade de serviço externo nesta etapa.

O usuário pediu os documentos em `docs/planning`; este ciclo usa arquivos locais
como tracker, com um ticket por arquivo e triagem `ready-for-agent`. Esse destino
explícito substitui o diretório padrão `.scratch` de `to-tickets`. A fronteira de testes
e a esteira foram aprovadas; os tickets individuais estão no índice de planejamento.
Publicação em GitHub/Linear não foi executada e, se solicitada, requer configuração
do destino externo. Não é necessário configurar um serviço externo para usar estes documentos.

Referências consultadas para a proposta:

- [Qt: integração tray e disponibilidade](https://doc.qt.io/qt-6.12/qsystemtrayicon.html)
- [Apple: menus e ações](https://developer.apple.com/design/human-interface-guidelines/menus-and-actions)
- [Qt: deployment macOS](https://doc.qt.io/qt-6/macos-deployment.html)
- [Apple: distribuição macOS](https://developer.apple.com/documentation/xcode/packaging-mac-software-for-distribution)
- [AppImage: base de compatibilidade](https://docs.appimage.org/reference/best-practices.html)
- [Inno Setup](https://jrsoftware.org/isinfo.php)
