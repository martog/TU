package com.example.cryptocompare;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ImageView;

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


public class MainActivity extends AppCompatActivity {
    private String apiKey = BuildConfig.CCApiKey;
    private SearchableSpinner fromSpinner;
    private SearchableSpinner toSpinner;
    private ImageView btnSwitch;
    private ArrayAdapter<String> adapterData;
    private View iconAction;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main_activity);
        fromSpinner = findViewById(R.id.spinner1);
        toSpinner = findViewById(R.id.spinner2);
        btnSwitch = findViewById(R.id.btnSwitch);
        iconAction = findViewById(R.id.iconAction);
        btnSwitch.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (adapterData != null && fromSpinner.getSelectedItem() != null
                        && toSpinner.getSelectedItem() != null) {
                    switchSpinners();
                }
            }
        });
        iconAction.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.i("ItemIcon", "clicked!");
            }
        });
        spinnersInit();
    }

    public void spinnersInit() {
        final List<String> coins = new ArrayList<String>();
        adapterData = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, coins);
        fromSpinner.setAdapter(adapterData);
        fromSpinner.setTitle("From");
        fromSpinner.setPositiveButton("OK");
        toSpinner.setAdapter(adapterData);
        toSpinner.setTitle("To");
        toSpinner.setPositiveButton("OK");
        JsonObjectRequest allCoins = new JsonObjectRequest(Request.Method.GET, getResources().getString(R.string.all_coins_url), null, new Response.Listener<JSONObject>() {

            @Override
            public void onResponse(JSONObject response) {
                try {
                    JSONObject data = response.getJSONObject("Data");
                    Iterator<String> iter = data.keys();
                    while (iter.hasNext()) {
                        String key = iter.next();
                        coins.add(key);
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

    private void switchSpinners() {
        String from = (String) fromSpinner.getSelectedItem();
        String to = (String) toSpinner.getSelectedItem();
        fromSpinner.setSelection(adapterData.getPosition(to));
        toSpinner.setSelection(adapterData.getPosition(from));
    }
}
