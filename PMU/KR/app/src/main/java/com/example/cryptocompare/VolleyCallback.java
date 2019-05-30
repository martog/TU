package com.example.cryptocompare;

import org.json.JSONObject;

public interface VolleyCallback {
    void onSuccessResponse(JSONObject result, String to, Double value);
}
