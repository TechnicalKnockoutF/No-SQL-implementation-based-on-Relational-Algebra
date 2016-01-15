import org.json.simple.JSONObject;

import java.io.FileReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.json.simple.JSONArray;
import org.json.simple.parser.ParseException;
import org.json.simple.parser.JSONParser;

public class JSONRead {

	static double padding = 0.1;
	static double materialCost = 0.75;
	static double laserSpeed = 0.5;
	static double machineCost = 0.07;
	
	static HashMap<Long, Vertices> vertexMap = new HashMap<Long, Vertices>();
	static Edges edgeObj = new Edges();
	
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		ReadJSON("C://Users//susan//workspace//Plethora//src//Rectangle.json");
		ProduceQuote();
		
		vertexMap = new HashMap<Long, Vertices>();
		edgeObj = new Edges();
		ReadJSON("C://Users//susan//workspace//Plethora//src//ExtrudeCircularArc.json");
		ProduceQuote();
		
		vertexMap = new HashMap<Long, Vertices>();
		edgeObj = new Edges();
		ReadJSON("C://Users//susan//workspace//Plethora//src//CutCircularArc.json");
		ProduceQuote();
	}
	
	private static void ProduceQuote() {
		// TODO Auto-generated method stub
		HashMap<String, ArrayList<Object>> edgeMap = edgeObj.getEdgeMap(); //EdgeType and Collection of edges
		double totalMatCost =0;
		double totalMachineCost = 0;
		if(edgeMap.containsKey("CircularArc"))
		{
			double[] dimension = edgeObj.getAllDimension();
			double[] sides = edgeObj.getDimension();
			double radius = edgeObj.getRadius();
			
			String circleType = edgeObj.getCircleType();//internal or external
			
			if(circleType.equals("Internal"))
			{
				totalMatCost = (dimension[0]+padding)*(dimension[1]+padding)*materialCost;
			}
			else
			{
				if(dimension[0] == radius)
				{
					totalMatCost = (((dimension[0])*(dimension[1]+padding)) + (((2*radius)+padding)*(radius+padding)))*materialCost;
				}
				else
				{
					totalMatCost = (((dimension[1])*(dimension[0]+padding)) + (((2*radius)+padding)*(radius+padding)))*materialCost;
				}				
			}
			
			totalMachineCost = (((sides[0]+sides[1]+sides[2])) + (Math.PI*radius/Math.exp((-1)/radius)))/laserSpeed*machineCost;
		}
		else
		{
			double[] dimension = edgeObj.getAllDimension();

			totalMatCost = (dimension[0]+padding)*(dimension[1]+padding)*materialCost;
			totalMachineCost = (2*(dimension[0]+dimension[1]))/laserSpeed*machineCost;
		}
//		System.out.println(totalMatCost + totalMachineCost);
		System.out.println((double)(Math.round((totalMatCost + totalMachineCost)*100))/100);
	}

	public static void ReadJSON(String filename)
	{
		JSONParser jp = new JSONParser();
		try {
			Object obj = jp.parse(new FileReader(filename));
			JSONObject jo = (JSONObject) obj;
			JSONObject vertices = (JSONObject) jo.get("Vertices");
			
			for(Object o : vertices.entrySet())
			{
				String objO = o.toString();
				String id = objO.substring(0, objO.indexOf("="));
				
				JSONObject vertex = (JSONObject) vertices.get(id);
				JSONObject position = ((JSONObject) vertex.get("Position"));
				
				double X = (double) position.get("X");
				double Y = (double) position.get("Y");
				
				Vertices vert = new Vertices(Long.parseLong(id), X, Y);
				vertexMap.put(Long.parseLong(id), vert);
			}
			
			JSONObject edges = (JSONObject) jo.get("Edges");
			
			for(Object o : edges.entrySet())
			{
				Vertices[] vertObj = new Vertices[2];
				String objO = o.toString();
				String id = objO.substring(0, objO.indexOf("="));
				
				JSONObject edge = (JSONObject) edges.get(id);
				String type = ((String) edge.get("Type"));
				
				JSONArray vertex = ((JSONArray) edge.get("Vertices"));
				Iterator<Long> iterator = ((List) vertex).iterator();
				int count = 0;
				while (iterator.hasNext()) 
				{
					Vertices v = vertexMap.get(iterator.next());
					vertObj[count++] = v ;
				}
				
				if(type.equals("LineSegment"))
				{
					
					LineSegment line = new LineSegment(Long.parseLong(id), vertObj);

					edgeObj.setEdgeMap(type, line);
				}				
				else
				{
					JSONObject position = ((JSONObject) edge.get("Center"));
					
					double X = (double) position.get("X");
					double Y = (double) position.get("Y");
					
					CircularArc circle = new CircularArc(Long.parseLong(id), vertObj, X, Y);
					circle.setStartVert(vertexMap.get((long) edge.get("ClockwiseFrom")));

					edgeObj.setEdgeMap(type, circle);
				}
			}


			System.out.println();
		} catch (Exception e) {
			e.printStackTrace();
		}

	}
}
