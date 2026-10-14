# 03: Espelhar e iniciar com gravação pela janela compacta

**ID:** N03

**What to build:** Escolher um celular, espelhar ou iniciar com gravação, acompanhar o estado, capturar, encerrar e recuperar falhas pela janela compacta.

**Blocked by:** 02 / N02 — Executar uma ação por menu e toolbar com as mesmas regras.

**Status:** in-progress

**Implementação:** piloto de display com motor real autorizado pelo usuário. Seleção, start/stop, MKV no lançamento, PNG e envio explícito ligados ao controlador; migração completa, persistência/modos e prova hardware pendentes. Workspace principal preservado, gates N01/N02 ainda abertos.

**Evidência hardware parcial:** usuário confirmou “funcionou conforme esperado” após
experimentar espelhamento nesta máquina macOS. Não extrapolar para gravação/playback,
captura/transferência, comandos de imagem ou outras plataformas.

**Origem / reconciliação:** L01–L03, L05, L06, L14–L16 e L25; migração da apresentação, não reimplementação dos recursos.

## Acceptance criteria

- [ ] Dispositivo e transporte aparecem claramente; a sessão só inicia por ação explícita e readiness vem do motor.
- [ ] Iniciar com gravação oferece formato válido antes do lançamento; não apresenta gravação ao vivo como recurso implementado.
- [ ] Parar usa a finalização existente, reportando sucesso ou interrupção honestamente; falha e disconnect permitem nova tentativa.
- [ ] Ações já implementadas continuam alcançáveis em menus ou janelas sob demanda, incluindo mídia real e diagnósticos.
- [ ] Trocar dispositivo/configuração de próxima sessão não altera o alvo nem a política da sessão ativa.
- [ ] Remover apresentação antiga somente após comprovar as jornadas migradas e registrar o destino dos controles.

## Verification

Jornada pelo controlador e interface com helper; captura, stop e arquivo reproduzível em telefone real.
