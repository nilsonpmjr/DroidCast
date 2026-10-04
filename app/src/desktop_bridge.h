#ifndef SC_DESKTOP_BRIDGE_H
#define SC_DESKTOP_BRIDGE_H

// Private, opt-in desktop protocol. Ordinary CLI invocations are unaffected.
void sc_desktop_bridge_start(void);
void sc_desktop_bridge_stop(void);
void sc_desktop_bridge_report(const char *event);
void sc_desktop_bridge_first_frame(void);
void sc_desktop_bridge_camera_controls_ready(void);

#endif
