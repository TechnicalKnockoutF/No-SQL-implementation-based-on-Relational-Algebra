import java.util.ArrayList;
import java.util.HashMap;

public class Edges {
	
	private HashMap<String, ArrayList<Object>> edgeMap = new HashMap<String, ArrayList<Object>>();
	
	public Edges()
	{
		
	}

	public HashMap<String, ArrayList<Object>> getEdgeMap() {
		return edgeMap;
	}
	
	public void setEdgeMap(String edgeType, Object edge)
	{
		
		if(getEdgeMap().containsKey(edgeType))
		{
			ArrayList<Object> temp = getEdgeMap().get(edgeType);
			temp.add(edge);
			getEdgeMap().put(edgeType, temp);
		}
		else
		{
			ArrayList<Object> temp = new ArrayList<Object>();
			temp.add(edge);
			getEdgeMap().put(edgeType, temp);
		}
		
	}
	
	public String getCircleType()
	{
		double centerX = ((CircularArc) edgeMap.get("CircularArc").get(0)).getCenterX();
		double centerY = ((CircularArc) edgeMap.get("CircularArc").get(0)).getCenterY();
		
		double startX = ((CircularArc) edgeMap.get("CircularArc").get(0)).getStartVert().getX();
		double startY = ((CircularArc) edgeMap.get("CircularArc").get(0)).getStartVert().getY();
		
		ArrayList<Object> line = edgeMap.get("LineSegment");
		int count = 0;
		/*
		 * A -- Bottom Left
		 * B -- Top Left
		 * C -- Top Right
		 * D -- Bottom Right
		 */
		String vertexName = "";
		for(Object o: line)
		{
			Vertices[] vert = ((LineSegment) o).getVert();
			for(int i =0; i<2; i++ )
			{
				if(startX < vert[i].getX() && startY < vert[i].getY())
				{
					vertexName ="A";
					break;
				}
				if(startX > vert[i].getX() && startY > vert[i].getY())
				{
					vertexName ="C";
					break;
				}
				if(startX < vert[i].getX() && startY > vert[i].getY())
				{
					vertexName ="B";
					break;
				}
				if(startX > vert[i].getX() && startY < vert[i].getY())
				{
					vertexName ="D";
					break;
				}
			}
			
			if(vertexName.equals(""))
			{
				
			}
			else
			{
				break;
			}
		}
		
		if(vertexName.equals("A"))
		{
			if(startX == centerX)
			{
				return "External";
			}
			else
			{
				return "Internal";
			}
		}
		
		if(vertexName.equals("B"))
		{
			if(startX == centerX)
			{
				return "Internal";
			}
			else
			{
				return "External";
			}
		}
		
		if(vertexName.equals("C"))
		{
			if(startX == centerX)
			{
				return "External";
			}
			else
			{
				return "Internal";
			}
		}
		
		if(vertexName.equals("D"))
		{
			if(startX == centerX)
			{
				return "Internal";
			}
			else
			{
				return "External";
			}
		}
		return null;
	}
	
	public double getRadius()
	{
		return ((CircularArc) edgeMap.get("CircularArc").get(0)).getRadius();
	}
	
	public double[] getAllDimension()
	{
		double[] dimension = new double[2];
		
		double[] sides = new double[4];
		ArrayList<Object> line = edgeMap.get("LineSegment");
		int count = 0;
		for(Object o: line)
		{
			sides[count++]=((LineSegment) o).getLength();
		}
		
		dimension[0] = sides[0];
		
		for(int i=1; i < sides.length; i++)
		{
			if((i == sides.length -1))
			{
				if(sides[i] == 0)
				{
					dimension[1] = dimension[0];
				}
			}
			else if(sides[0] != sides[i])
			{
				dimension[1] = sides[i];
				break;
			}
		}
		
		return dimension;
	}

	public double[] getDimension() {
		// TODO Auto-generated method stub
		double[] sides = new double[4];
		ArrayList<Object> line = edgeMap.get("LineSegment");
		int count = 0;
		for(Object o: line)
		{
			sides[count++]=((LineSegment) o).getLength();
		}
		
		return sides;
	}
}
