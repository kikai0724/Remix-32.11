#pragma once

inline constexpr int Port = 7777;
inline bool bReload = true;
inline bool bEvent = false;
inline bool bDuo = false;
inline std::wstring ReloadPlaylist = L"/BlastBerry/Playlists/Playlist_PunchBerrySolo.Playlist_PunchBerrySolo";
inline std::wstring Playlist = bEvent ? L"/QuailPlaylist/Playlist/Playlist_Quail.Playlist_Quail"
    : bDuo ? L"/BRPlaylists/Athena/Playlists/Playlist_DefaultDuo.Playlist_DefaultDuo"
    : bReload ? ReloadPlaylist : L"/BRPlaylists/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo";
inline bool bInit = false;
inline bool bReady = false;
inline bool bEnableZones = true;
inline bool bForceRespawns = false;
inline bool bStormRespawns = false;
inline bool bLateGame = false;
inline int LateGameZone = 3;
inline bool bLateGameLongZone = false;
inline bool bLateGameMovingBus = true;
inline std::vector<std::pair<std::wstring, int>> LateGameLoadout;
inline bool bRecordReplays = true;
inline bool bPlayerBots = true;
inline bool bAutoRestart = true;
inline std::wstring BackendUrl;
inline std::wstring ClientId;
inline std::wstring ClientSecret;
