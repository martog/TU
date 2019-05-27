package com.example.cryptocompare;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import com.android.volley.AuthFailureError;
import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.JsonObjectRequest;
import com.android.volley.toolbox.Volley;
import com.toptoche.searchablespinnerlibrary.SearchableSpinner;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import javax.security.auth.login.LoginException;


public class MainActivity extends AppCompatActivity {
    private String apiKey = BuildConfig.CCApiKey;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        fromSpinnerInit();

    }

    public void fromSpinnerInit() {
        final SearchableSpinner fromSpinner = findViewById(R.id.fromSpinner);
        final List<String> coins = new ArrayList<String>();
        final ArrayAdapter<String> dataAdapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, coins);
        fromSpinner.setAdapter(dataAdapter);
        fromSpinner.setTitle("From");
        fromSpinner.setPositiveButton("OK");
        JsonObjectRequest allCoins = new JsonObjectRequest(Request.Method.GET, getResources().getString(R.string.all_coins_url), null, new Response.Listener<JSONObject>() {

            @Override
            public void onResponse(JSONObject response) {
                try {
                    JSONObject data = response. getJSONObject("Data");
                    Iterator<String> iter = data.keys();
                    while (iter.hasNext()) {
                        String key = iter.next();
                        try {
                            JSONObject value = data.getJSONObject(key);
                            coins.add(value.get("CoinName").toString() + " (" + value.get("Name").toString() + ")");
                        } catch (JSONException e) {
                            // Something went wrong!
                        }
                    }

                } catch (JSONException e) {
                    Log.i("Response", e.toString());
                }

            }
        }, new Response.ErrorListener() {

                    @Override
                    public void onErrorResponse(VolleyError error) {
                        Log.i("ResponseError", error.toString());

                    }
                }) {
            @Override
            public Map<String, String> getHeaders() throws AuthFailureError {
                Map<String, String> params = new HashMap<String, String>();
                params.put("Apikey", apiKey);
                return params;
            }
        };
        Volley.newRequestQueue(this).add(allCoins);

    }
}
