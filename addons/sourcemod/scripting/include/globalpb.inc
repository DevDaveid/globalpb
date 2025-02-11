#include <steamworks>

stock const char gC_APIModes[3][] =
{
	"kz_vanilla",
	"kz_simple",
	"kz_timer"
};

stock const char gC_ModeShort[3][] =
{
	"VNL",
	"SKZ",
	"KZT"
};

stock const char gC_JumpTypes[7][] = 
{
	"LongJump", 
	"Bhop", 
	"MultiBhop", 
	"WeirdJump", 
	"DropBhop", 
	"CountJump", 
	"LadderJump"
};

stock bool RequestGlobalPB(int client, const char[] map, int course, int mode, bool teleports, SteamWorksHTTPRequestCompleted callback, any data1 = 0, any data2 = 0)
{
	char steamid[64];
	GetClientAuthId(client, AuthId_Steam2, steamid, sizeof(steamid));

	char link[512];
	Format(link, sizeof(link), 
		"https://kztimerglobal.com/api/v1.0/records/top?steam_id=%s&map_name=%s&stage=%d&modes_list_string=%s&has_teleports=%s&limit=1&tickrate=128", 
		steamid, map, course, gC_APIModes[mode], teleports ? "true" : "false");

	Handle request = SteamWorks_CreateHTTPRequest(k_EHTTPMethodGET, link);
	if (request != null)
	{
		SteamWorks_SetHTTPRequestContextValue(request, data1, data2);
		if (!SteamWorks_SetHTTPCallbacks(request, callback) || !SteamWorks_SendHTTPRequest(request))
		{
			delete request;
			return false;
		}
	}
	return true;
}

stock bool SteamWorks_GetHTTPResponseBody_Easy(Handle request, char[] buffer, int maxlength)
{
	int size;
	if (SteamWorks_GetHTTPResponseBodySize(request, size) && (maxlength > size))
	{
		if (SteamWorks_GetHTTPResponseBodyData(request, buffer, size))
		{
			return true;
		}
	}
	return false;
}

stock bool GetRequestRecordInfo(Handle request, float& time, int& teleports)
{
	time = -1.0;
	teleports = -1;

	char buffer[2048];
	if (!SteamWorks_GetHTTPResponseBody_Easy(request, buffer, sizeof(buffer)))
	{
		return false;
	}

	Handle json = json_load(buffer);
	if (json == null)
	{
		return false;
	}

	Handle obj = json_array_get(json, 0);
	if (obj == null)
	{
		delete json;
		return true;
	}

	Handle objTime = json_object_get(obj, "time");
	Handle objTeleports = json_object_get(obj, "teleports");

	if (objTime != null && objTeleports != null)
	{
		time = json_real_value(objTime);
		teleports = json_integer_value(objTeleports);
	}

	delete objTeleports;
	delete objTime;
	delete obj;
	delete json;
	return true;
}

stock int FormatDuration(char[] buffer, int maxlength, float duration)
{
    int hours = RoundToFloor(duration / 3600.0);
    duration -= hours * 3600;
    int minutes = RoundToFloor(duration / 60.0);
    duration -= minutes * 60;
    int seconds = RoundToFloor(duration);
    duration -= seconds;
    int milliseconds = RoundToFloor(duration * 100.0);

    if (hours > 0)
        return Format(buffer, maxlength, "%02d:%02d:%02d.%02d", hours, minutes, seconds, milliseconds);
    else if (minutes > 0)
        return Format(buffer, maxlength, "%02d:%02d.%02d", minutes, seconds, milliseconds);

    return Format(buffer, maxlength, "%02d.%02d", seconds, milliseconds);
}

stock bool RequestGlobalJS(int client, const char[] jumpType, SteamWorksHTTPRequestCompleted callback, any data1 = 0, any data2 = 0)
{
    char steamid[64], link[512];
    GetClientAuthId(client, AuthId_Steam2, steamid, sizeof(steamid));

    Format(link, sizeof(link), "https://kztimerglobal.com/api/v1.0/jumpstats/%s/top?steam_id=%s", jumpType, steamid);
    Handle request = SteamWorks_CreateHTTPRequest(k_EHTTPMethodGET, link);

    if(request != null)
    {
        SteamWorks_SetHTTPRequestContextValue(request, data1, data2);
        if(!SteamWorks_SetHTTPCallbacks(request, callback) || !SteamWorks_SendHTTPRequest(request))
        {
            delete request;
            return false;
        }
    }
    return true;
}

stock bool GetRequestGlobalJSInfo(Handle request, float& distance, int& strafe_count, int& isBinded, char[] cDate)
{
    distance = 0.0;
    strafe_count = 0;
    isBinded = 0;

    char buffer[2048];
    if (!SteamWorks_GetHTTPResponseBody_Easy(request, buffer, sizeof(buffer)))
    {
        return false;
    }

    Handle json = json_load(buffer);
    if(json == null)
    {
        return false;
    }

    Handle obj = json_array_get(json, 0);
    if(obj == null)
    {
        delete json;
        return true;
    }

    Handle objDistance = json_object_get(obj, "distance");
    Handle objStrafes = json_object_get(obj, "strafe_count");
	Handle objBinded = json_object_get(obj, "is_crouch_boost");
	Handle objDate = json_object_get(obj, "created_on");
	
	if(objDistance != null && objStrafes != null)
    {
        distance = json_real_value(objDistance);
        strafe_count = json_integer_value(objStrafes);
        isBinded = json_integer_value(objBinded);
		json_string_value(objDate, cDate, 64);
    }

	delete objDistance;
    delete objStrafes;
    delete objBinded;
	delete objDate;
	delete obj;
	delete json;
    return true;
}
