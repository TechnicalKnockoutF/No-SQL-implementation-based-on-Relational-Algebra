
public class Vertices {

	private Long id;
	private double x;
	private double y;
	
	public Vertices()
	{
		
	}
	
	public Vertices(Long vertID, double xAxisPt, double yAxisPt)
	{
		setId(vertID);
		setX(xAxisPt);
		setY(yAxisPt);
	}

	public Long getId() {
		return id;
	}

	public void setId(Long id) {
		this.id = id;
	}

	public double getX() {
		return x;
	}

	public void setX(double x) {
		this.x = x;
	}

	public double getY() {
		return y;
	}

	public void setY(double y) {
		this.y = y;
	}
	
	
}
