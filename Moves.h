

// These are all members of CPhysicsObj

void Movement_Init();
void Movement_Shutdown();
void Movement_Think();

void Movement_UpdatePos();
void Movement_SendUpdate(DWORD dwCell);

void Movement_Teleport(loc_t origin, heading_t angles);

double m_fMoveThink;

loc_t m_lastMoveOrigin;
heading_t m_lastMoveAngles;