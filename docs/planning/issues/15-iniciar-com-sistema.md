# 15: Iniciar opcionalmente na sessão do usuário

**ID:** N18

**What to build:** Habilitar ou desabilitar Iniciar com o sistema para disponibilizar DroidCast e sua oferta USB na sessão do usuário, com lançamento em segundo plano apenas quando recuperável.

**Blocked by:** 08 / N08 — Política de fechar/sair; 14 / N17 — Oferta por conexão USB.

**Status:** ready-for-agent

**Implementação:** não iniciada.

**Origem / reconciliação:** Novo comportamento explicitado pelo usuário; estende lifecycle N08 e deve ser validado nos pacotes L08–L10 sem configurar a máquina atual durante planejamento.

## Acceptance criteria

- [ ] Preferência desabilitada inicialmente, visível, reversível e aplicada por mecanismo suportado da sessão do usuário em cada plataforma; erro de registro não aparece como sucesso.
- [ ] Lançamento de login pode ficar em segundo plano somente com caminho de restauração disponível; sem tray/capacidade adequada, abre janela recuperável. Lançamento manual abre janela.
- [ ] Não registrar serviço privilegiado, observador USB separado ou ativação por plug depois de Sair; login não inicia espelhamento/envio automaticamente.
- [ ] Uma única instância controla discovery/sessão; execução duplicada não gera ofertas ou processos duplicados.
- [ ] Desabilitar remove somente o registro próprio; upgrade mantém caminho válido e uninstall remove somente sua integração de login, preservando dados do usuário.
- [ ] Sair interrompe monitoramento na sessão atual sem apagar a preferência para o próximo login.
- [ ] Integração final é testada com DMG/EXE/AppImage onde suportada; dependências de empacotamento são verificações de release, não bloqueios para desenvolver o opt-in.

## Verification

Testar habilitar → login → conectar USB → escolher ação → Sair → reconnect sem oferta → próximo login → desabilitar. Validar ausência de privilégios adicionais, restauração sem tray, execução duplicada e upgrade/uninstall em ambientes de teste, sem alterar o autostart da máquina de desenvolvimento por consequência deste ticket.
