#pragma once
class Random
{
private:
    ConfigLoader* m_configLoader;
    MyDisplay* m_display;
    Level* m_level;
    LocalPlayer* m_localPlayer;
    std::vector<Player*>* m_players;
public:
    Random(ConfigLoader* configLoader,
          MyDisplay* display,
          Level* level,
          LocalPlayer* localPlayer,
          std::vector<Player*>* players
          )
    {
        m_configLoader = configLoader;
        m_display = display;
        m_level = level;
        m_localPlayer = localPlayer;
        m_players = players;
    }
    //==========---------[SUPERGLIDE]-------------==================
    void superGlide(){
        while (m_display->keyDown(XK_space))
        {
            static float startjumpTime = 0;
            static bool startSg = false;
            static float traversalProgressTmp = 0.0;
 
            float worldtime = mem::Read<float>(m_localPlayer->base + OFF_TIME_BASE); // Current time
            float traversalStartTime = mem::Read<float>(m_localPlayer->base + OFF_TRAVERSAL_START_TIME); // Time to start wall climbing
            float traversalProgress = mem::Read<float>(m_localPlayer->base + OFF_TRAVERSAL_PROGRESS); // Wall climbing, if > 0.87 it is almost over.
            auto HangOnWall = -(traversalStartTime - worldtime);
 
            if (HangOnWall > 0.1 && HangOnWall < 0.12)
            {
                mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 4);
            }
            if (traversalProgress > 0.87f && !startSg && HangOnWall > 0.1f && HangOnWall < 1.5f)
            {
                //start SG
                startjumpTime = worldtime;
                startSg = true;
            }
            if (startSg)
            {
                //printf ("sg Press jump\n");
                mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 5);
                while (mem::Read<float>(m_localPlayer->base + OFF_TIME_BASE) - startjumpTime < 0.011);
                {
                    mem::Write<int>(OFF_REGION + OFF_IN_DUCK + 0x8, 6);
                    std::this_thread::sleep_for(std::chrono::milliseconds(30));
                    mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 4);
                    std::this_thread::sleep_for(std::chrono::milliseconds(600));
                }
                startSg = false;
                break;
            }
        }
 
        // Automatic wall jump
        int wallJumpNow = 0;
 
        static float onWallTmp = 0;
        float onWall = mem::Read<float>(m_localPlayer->base + OFF_WALL_RUN_START_TIME);
        if (onWall > onWallTmp + 0.1) // 0.1
        {
            if (mem::Read<int>(OFF_REGION + OFF_IN_FORWARD) == 0)
            {
                wallJumpNow = 1;
                //printf("wall jump Press jump\n");
                mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 5);
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 4);
            }
        }
        onWallTmp = onWall;
 
        static float onEdgeTmp = 0;
        float onEdge = mem::Read<float>(m_localPlayer->base + OFF_TRAVERSAL_PROGRESS);
        if (onEdge > onEdgeTmp + 0.1) // 0.1
        {
            if (mem::Read<int>(OFF_REGION + OFF_IN_FORWARD) == 0)
            {
                wallJumpNow = 2;
                //printf("wall jump onEdge Press jump\n");
                mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 5);
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                mem::Write<int>(OFF_REGION + OFF_IN_JUMP + 0x8, 4);
            }
        }
        onEdgeTmp = onEdge;    
    }

    //==========---------[QUICKTURN]-------------==================
    void quickTurn()
    {
        Vector2D localYawtoClamp = m_localPlayer->viewAngles;
        localYawtoClamp.Clamp();
        float localYaw = localYawtoClamp.y;
        // quickTurn
        if(m_configLoader->FEATURE_QUICKTURN_ON){
            if(m_display->keyDown(m_configLoader->FEATURE_QUICKTURN_BUTTON)){
                m_localPlayer->setYaw((localYaw + 180));
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
    }

    //====================---------[PRINTLEVELS]-------------------------==================
    void printLevels()
    {
        if(m_configLoader->FEATURE_PRINT_LEVELS_ON){

            if(m_display->keyDown(m_configLoader->FEATURE_PRINT_LEVELS_BUTTON)){
                printf("[N]=[NAME]-[LEVEL]-[LEGEND]\n\n");
                for (auto i = 0; i < m_players->size(); i++)
                {
                    Player *p = m_players->at(i);
                    if(!p->dead && p->isPlayer()){
                        int playerLvl = p->GetPlayerLevel();
                        std::string namePlayer = p->getPlayerName();
                        std::string modelName = p->getPlayerModelName();
                        
                        if(p->friendly){
                            printf("\033[91m[%i]=[%s]-[%i]-[%s]\033[0m\n",
                            (i+1), namePlayer.c_str(), playerLvl, modelName.c_str());  
                        }else{
                            printf("[%i]=[%s]-[%i]-[%s]\n",
                            (i+1), namePlayer.c_str(), playerLvl, modelName.c_str());
                        }
                    }           
                }   
                printf("\n");
            }            
        }        
    }

    //==================---------------[SPECTATORVIEW]-------------------==================
    void spectatorView()
    {
        if(!m_level->playable) return;
        int spectatorcount = 0;   
        std::vector<std::string> spectatorlist;
        if(m_configLoader->FEATURE_SPECTATOR_ON){
            for (int i = 0; i < m_players->size(); i++)
            { 
                Player *p = m_players->at(i);          
                      
                float targetyaw = p->view_yaw; // get Yaw player
                float localyaw= m_localPlayer->local_yaw; // get Yaw LocalPlayer
                //printf("LocalViewYaw: %f == PlayerViewYaw: %f \n", localyaw, targetyaw);
                if (targetyaw == localyaw && p->currentHealth == 0){
                    spectatorcount++;
                    
                    std::string namePlayer = p->getPlayerName();    
                    spectatorlist.push_back(namePlayer);
                }            
            }
            const auto spectatorlist_size = static_cast<int>(spectatorlist.size());
           
            if (spectatorcount > 0){
                {   
                    printf("\n-[%d]-- SPECTATORS -- \n", spectatorcount);
                    for (int i = 0; i < spectatorlist_size; i++)   //show list of spectators by name
                    {   
                        printf("---[%s]---\n", spectatorlist.at(i).c_str());
                    }
                }
            }              
        }      
    }

    //==================---------[SKINCHANGER]-------------==========================================
    void SkinChange()
    {
        if(!m_level->playable) return;
        if(m_localPlayer->dead) return;
        long wephandle = mem::Read<long>(m_localPlayer->base + OFF_WEAPON_HANDLE);
        wephandle &= 0xffff;
        long wep_entity = m_localPlayer->weaponEntity;
        float curTime = mem::Read<float>(m_localPlayer->base + OFF_TIME_BASE);
        float endTime = curTime +5.5;
        std::map<int, std::vector<int>> weaponSkinMap;
        //Light ammo weapons
        weaponSkinMap[WEAPON_P2020] = { 6 };   //WEAPON_P2020 
        weaponSkinMap[WEAPON_RE45] = { 6 };   //WEAPON_RE45 
        weaponSkinMap[WEAPON_ALTERNATOR] = { 11 };   //WEAPON_ALTERNATOR 
        weaponSkinMap[WEAPON_R99] = { 2 };   //WEAPON_R99  
        weaponSkinMap[WEAPON_R301] = { 10 };     //WEAPON_R301   
        weaponSkinMap[WEAPON_SPITFIRE] = { 2 };    //WEAPON_SPITFIRE 
        weaponSkinMap[WEAPON_G7] = { 5 };    //WEAPON_G7 
        //Heavy ammo weapons
        weaponSkinMap[WEAPON_CAR] = { 10};   // Car-SMG 
        weaponSkinMap[WEAPON_RAMPAGE] = { 6 };    // Rampage 
        weaponSkinMap[WEAPON_3030] = { 9 };      //3030 
        weaponSkinMap[WEAPON_HEMLOCK] = {10 };   //WEAPON_HEMLOCK  
        weaponSkinMap[WEAPON_FLATLINE] = { 8 };    //FlatLine  
        //Energy ammo weapons
        weaponSkinMap[WEAPON_NEMESIS] = { 8 };    //WEAPON_NEMESIS  
        weaponSkinMap[WEAPON_VOLT] = { 9 };    //WEAPON_VOLT 
        weaponSkinMap[WEAPON_TRIPLE_TAKE] = { 7 };    //WEAPON_TRIPLE_TAKE 
        weaponSkinMap[WEAPON_LSTAR] = { 3 };    //WEAPON_LSTAR 
        weaponSkinMap[WEAPON_DEVOTION] = { 5 };    //WEAPON_DEVOTION 
        weaponSkinMap[WEAPON_HAVOC] = { 8 };    //WEAPON_HAVOC 
        //Sniper ammo weapons
        weaponSkinMap[WEAPON_SENTINEL] = { 5 };    //WEAPON_SENTINEL 
        weaponSkinMap[WEAPON_CHARGE_RIFLE] = { 8 };    //WEAPON_CHARGE_RIFLE 
        weaponSkinMap[WEAPON_LONGBOW] = { 7 };    //WEAPON_LONGBOW 
        //Shotgun ammo weapons
        weaponSkinMap[WEAPON_MOZAMBIQUE] = { 5 };    //WEAPON_MOZAMBIQUE 
        weaponSkinMap[WEAPON_EVA8] = { 8 };    //WEAPON_EVA8 
        weaponSkinMap[WEAPON_PEACEKEEPER] = { 7 };    //WEAPON_PEACEKEEPER 
        weaponSkinMap[WEAPON_MASTIFF] = { 5 };    //WEAPON_MASTIFF 
        //Legendary ammo weapons
        weaponSkinMap[WEAPON_WINGMAN] = { 5 };    //WEAPON_WINGMAN 
        weaponSkinMap[WEAPON_PROWLER] = { 7 };    //WEAPON_PROWLER
        weaponSkinMap[WEAPON_BOCEK] = { 3 };    //WEAPON_BOCEK
        weaponSkinMap[WEAPON_KRABER] = { 6 };    //WEAPON_KRABER
        weaponSkinMap[WEAPON_THROWING_KNIFE] = { 3 };    //WEAPON_THROWING_KNIFE
        weaponSkinMap[WEAPON_THERMITE_GRENADE] = { 2 };    //WEAPON_THERMITE_GRENADE 

        if (m_configLoader->FEATURE_SKINCHANGER_ON){
            int waponIndex = mem::Read<int>(wep_entity + OFF_WEAPON_INDEX);
            if (weaponSkinMap.count(waponIndex) == 0) return;
            int skinID = weaponSkinMap[waponIndex][0];
            //printf("Weapon: %s Activated Skin ID: %d \n", WeaponName(waponIndex).c_str(), skinID);  
            mem::Write<int>(m_localPlayer->base + OFF_SKIN, skinID+1);
            mem::Write<int>(wep_entity + OFF_SKIN, skinID);
            curTime = mem::Read<float>(m_localPlayer->base + OFF_TIME_BASE);
        }                    
    }
};
