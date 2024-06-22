package es.uma.rysd.app;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URL;
import java.net.URLEncoder;
import java.nio.charset.StandardCharsets;

import javax.net.ssl.HttpsURLConnection;

import com.google.gson.Gson;

import es.uma.rysd.entities.*;

public class SWClient {
	// TODO: Complete the application name
	private final String app_name = "API de Star Wars";
	private final int year = 2024;

	private final String url_api = "https://swapi.dev/api/";

	// Auxiliary methods provided

	// Gets the URL of the resource id of the type resource
	public String buildResourceUrl(String resource, Integer id){
		return url_api + resource + "/" + id + "/";
	}

	// Given a resource URL, gets its ID
	public Integer extractIdFromUrl(String url){
		String[] parts = url.split("/");

		return Integer.parseInt(parts[parts.length-1]);
	}

	// Queries a resource and returns how many elements it has
	public int countResources(String resource){
		// TODO: Handle possible exceptions appropriately
		try {
			
			// TODO: Create the corresponding URL: https://swapi.dev/api/{resource}/ replacing resource with the parameter
			String url = url_api + resource;
			
			// TODO: Create the connection from the URL
			URL servicio = new URL(url);
			
			// TODO: Add the headers User-Agent and Accept (see the statement)
			HttpsURLConnection connection = (HttpsURLConnection) servicio.openConnection();
			connection.setRequestProperty("Accept", "application/json");
			connection.setRequestProperty("User-Agent",  app_name+"-"+year);
				
			// TODO: Indicate that it is a GET request
			connection.setRequestMethod("GET");
			
			// TODO: Check that the response code received is correct
			int response = connection.getResponseCode();
			if (!(response >= 200 && response <= 299)) {
				System.err.format("Error: server response: %d: %s", response, connection.getResponseMessage());
				return (0);	
			}
			// TODO: Deserialize the response to ResourceCountResponse
			Gson parser = new Gson();
			InputStream in = connection.getInputStream(); // TODO: Get the InputStream from the connection
			ResourceCountResult c = parser.fromJson(new InputStreamReader(in), ResourceCountResult.class);
			
			// TODO: Return the number of elements
			return (c.count);
		} catch (IOException e) {
			throw new RuntimeException("Error: countResources" + e.getMessage());
		}
	}

	public Person getPerson(String urlname) {
			Person p = null;
			// Just in case it comes as http, we change it to https
			urlname = urlname.replaceAll("http:", "https:");
			
		// TODO: Handle possible exceptions appropriately
		try {
			
			// TODO: Create the connection from the received URL
			URL servicio = new URL(urlname);
			
			// TODO: Add the headers User-Agent and Accept (see the statement)
			HttpsURLConnection connection = (HttpsURLConnection) servicio.openConnection();
			connection.setRequestProperty("Accept", "application/json");
			connection.setRequestProperty("User-Agent",  app_name+"-"+year);

			// TODO: Indicate that it is a GET request
			connection.setRequestMethod("GET");
			
			// TODO: Check that the response code received is correct
			int response = connection.getResponseCode();
			if (!(response >= 200 && response <= 299)) {
				System.err.format("Error: server response: %d: %s", response, connection.getResponseMessage());
				return (null);	
			}
			
			// TODO: Deserialize the response to Person
			Gson parser = new Gson();
			InputStream in = connection.getInputStream(); // TODO: Get the InputStream from the connection

			p = parser.fromJson(new InputStreamReader(in), Person.class);
			
			// TODO: For questions 2 and 3 (do not need to complete this for question 1)
			// TODO: From the URL in the homeworld field, get the planet data and store it in the homeplanet attribute
			World world = getWorld(p.homeworld); 
			p.homeplanet = world;
			p.homeworld = world.name;
	
			return (p);
		} catch (Exception e) {
			throw new  RuntimeException("Error: getPerson()" + e.getMessage());
		}
	}

	public World getWorld(String urlname) {
		World p = null;
		// Just in case it comes as http, we change it to https
		urlname = urlname.replaceAll("http:", "https:");

		// TODO: Handle possible exceptions appropriately
		try {
				
			// TODO: Create the connection from the received URL
			URL servicio = new URL(urlname);

			// TODO: Add the headers User-Agent and Accept (see the statement)
			HttpsURLConnection connection = (HttpsURLConnection) servicio.openConnection();
			connection.setRequestProperty("Accept", "application/json");
			connection.setRequestProperty("User-Agent",  app_name+"-"+year);
			
			// TODO: Indicate that it is a GET request
			connection.setRequestMethod("GET");
			
			// TODO: Check that the response code received is correct
			int response = connection.getResponseCode();
			if (!(response >= 200 && response <= 299)) {
				System.err.format("Error: server response: %d: %s", response, connection.getResponseMessage());
				return (null);	
			}
			
			// TODO: Deserialize the response to Planet
			Gson parser = new Gson();
			InputStream in = connection.getInputStream(); // TODO: Get the InputStream from the connection
			p = parser.fromJson(new InputStreamReader(in), World.class);
			
			return (p);
		} catch (Exception e) {
			throw new RuntimeException("Error: getWorld()" + e.getMessage());
		}
	}

	public Person searchPersonByName(String name){
		Person p = null;
		// TODO: Handle possible exceptions appropriately
		try {	
			// TODO: Create the connection from the received URL
			URL servicio = new URL(url_api + "people/?search=" + URLEncoder.encode(name, StandardCharsets.UTF_8) + "/");

			// TODO: Add the headers User-Agent and Accept (see the statement)
			HttpsURLConnection connection = (HttpsURLConnection) servicio.openConnection();
			connection.setRequestProperty("Accept", "application/json");
			connection.setRequestProperty("User-Agent",  app_name+"-"+year);
			
			// TODO: Indicate that it is a GET request
			connection.setRequestMethod("GET");
			
			// TODO: Check that the response code received is correct
			int response = connection.getResponseCode();
			if (!(response >= 200 && response <= 299)) {
				System.err.format("Error: server response: %d: %s", response, connection.getResponseMessage());
				return (null);	
			}
						
			// TODO: Deserialize the response to SearchResponse -> Use the first position of the array as the result
			Gson parser = new Gson();
			InputStream in = connection.getInputStream(); // TODO: Get the InputStream from the connection
			QueryResponse qresponse = parser.fromJson(new InputStreamReader(in), QueryResponse.class);
			
			
			// TODO: For questions 2 and 3 (do not need to complete this for question 1)
			// TODO: From the URL in the homeworld field, get the planet data and store it in the homeplanet attribute
			if (qresponse.count > 0) {				
				p = qresponse.results[0];
				System.out.println("Hola panoli");
				World world = getWorld(p.homeworld);
				p.homeplanet = world;
				p.homeworld = world.name;
			}
	
			return (p);
		} catch (Exception e) {
			throw new RuntimeException("Error: getWorld()" + e.getMessage());
		}
	}

}
