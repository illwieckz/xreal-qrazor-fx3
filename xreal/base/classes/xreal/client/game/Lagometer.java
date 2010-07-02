package xreal.client.game;

import xreal.CVars;
import xreal.Color;
import xreal.UserCommand;
import xreal.client.Client;
import xreal.client.PlayerState;
import xreal.client.Snapshot;
import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;
import xreal.client.ui.Image;
import xreal.client.ui.Rectangle;
import xreal.client.ui.UserInterface;
import xreal.client.ui.Component;

/**
 * 
 * @author Robert Beckebans
 */
public class Lagometer extends Component {
	
	static private final int LAG_SAMPLES = 128;
	
	static private final int MAX_LAGOMETER_PING	= 900;
	static private final int MAX_LAGOMETER_RANGE = 300;
	
	//static private final int

	private int	frameSamples[] = new int[LAG_SAMPLES];
	private int	frameCount;
	private int	snapshotFlags[] = new int[LAG_SAMPLES];
	private int snapshotSamples[] = new int[LAG_SAMPLES];
	private int snapshotCount;
	
	private Image	lagometerPic;
	private Image	disconnectPic;
	private int		whiteMaterial;
	
	public Lagometer()
	{
		// place it into the lower right corner
		setX(UserInterface.SCREEN_WIDTH - 48);
		setY(UserInterface.SCREEN_HEIGHT - 48);
		setWidth(48);
		setHeight(48);
		
		lagometerPic = new Image("lagometer2");
		lagometerPic.setBounds(this.getBounds());
		
		disconnectPic = new Image("gfx/2d/net.tga");
		disconnectPic.setBounds(this.getBounds());
	
		
		whiteMaterial = Renderer.registerMaterialNoMip("white");
	}
	
	/**
	 * Adds the current interpolate / extrapolate bar for this frame
	 */
	void addFrameInfo(int latestSnapshotTime)
	{
		int             offset;

		offset = ClientGame.getTime() - latestSnapshotTime;
		frameSamples[frameCount & (LAG_SAMPLES - 1)] = offset;
		frameCount++;
	}

	
	/**
	 * Each time a snapshot is received, log its ping time and
	 * the number of snapshots that were dropped before it.
	 * 
	 * @param snap	Pass NULL for a dropped packet.
	 */
	void addSnapshotInfo(Snapshot snap)
	{
		// dropped packet
		if(snap == null)
		{
			snapshotSamples[snapshotCount & (LAG_SAMPLES - 1)] = -1;
			snapshotCount++;
			return;
		}

		// add this snapshot's info
		snapshotSamples[snapshotCount & (LAG_SAMPLES - 1)] = snap.getPing();
		snapshotFlags[snapshotCount & (LAG_SAMPLES - 1)] = snap.getSnapFlags();
		snapshotCount++;
	}
	
	/**
	 * Should we draw something different for long lag vs no packets?
	 */
	public void renderDisconnect()	
	{
		float           x, y;
		UserCommand     cmd;
		String			msg;
		int             w;

		// draw the phone jack if we are completely past our buffers
		cmd = Client.getOldestUserCommand();
		if(cmd.serverTime <= ClientGame.getSnapshotManager().getSnapshot().getPlayerState().commandTime || cmd.serverTime > ClientGame.getTime())
		{
			// special check for map_restart
			return;
		}

		// also add text in center of screen
		msg = "Connection Interrupted";
		ClientGame.getMedia().fontVera.paintText(UserInterface.SCREEN_WIDTH / 2, UserInterface.SCREEN_HEIGHT / 2, 16, Color.White, msg, 0, 0, Font.CENTER);

		// blink the icon
		if(((ClientGame.getTime() >> 9) & 1) != 0) {
			return;
		}
		
		disconnectPic.render();
	}
	
	public void render()
	{
		int             a, x, y, i;
		float           v;
		float			mid, range;
		int             color;
		float           vscale;
		boolean         lag = false;

		//PlayerState ps = ClientGame.getSnapshotManager().getSnapshot().getPlayerState();
		//cent = &cg_entities[cg.snap->ps.clientNum];

		// Tr3B: even draw the lagometer when connected to a local server
		if(!CVars.cg_lagometer.getBoolean()) //|| cgs.localServer  )
		{
			renderDisconnect();
			return;
		}
		
		
		Renderer.setColor(new Color(1.0f, 1.0f, 1.0f, 0.80f));
		lagometerPic.render();

		x = (int) Math.floor(bounds.x);
		y = (int) Math.floor(bounds.y);
		
		Rectangle rect = new Rectangle(x, y, 48, 48);
		UserInterface.adjustFrom640(rect);
		

		color = -1;
		range = rect.height / 3;
		mid = rect.y + range;

		vscale = range / MAX_LAGOMETER_RANGE;

		// draw the frame interpolate / extrapolate graph
		for(a = 0; a < rect.width; a++)
		{
			i = (frameCount - 1 - a) & (LAG_SAMPLES - 1);
			v = frameSamples[i];
			v *= vscale;
			if(v > 0)
			{
				if(color != 1)
				{
					color = 1;
					Renderer.setColor(Color.Yellow);
				}
				if(v > range)
				{
					v = range;
				}
				Renderer.drawStretchPic(rect.x + rect.width - a, mid - v, 1, v, 0, 0, 0, 0, whiteMaterial);
			}
			else if(v < 0)
			{
				if(color != 2)
				{
					color = 2;
					Renderer.setColor(Color.Blue);
				}
				v = -v;
				if(v > range)
				{
					v = range;
				}
				Renderer.drawStretchPic(rect.x + rect.width - a, mid, 1, v, 0, 0, 0, 0, whiteMaterial);
			}
		}

		// draw the snapshot latency / drop graph
		range = rect.height / 2;
		vscale = range / MAX_LAGOMETER_PING;

		for(a = 0; a < rect.width; a++)
		{
			i = (snapshotCount - 1 - a) & (LAG_SAMPLES - 1);
			v = snapshotSamples[i];
			if(v > 0)
			{
				if((snapshotFlags[i] & Snapshot.SNAPFLAG_RATE_DELAYED) != 0)
				{
					if(color != 5)
					{
						color = 5;	// YELLOW for rate delay
						Renderer.setColor(Color.Yellow);
					}
				}
				else
				{
					if(color != 3)
					{
						color = 3;
						Renderer.setColor(Color.Green);
					}
				}
				v = v * vscale;
				if(v > range)
				{
					v = range;
				}
				Renderer.drawStretchPic(rect.x + rect.width - a, rect.y + rect.height - v, 1, v, 0, 0, 0, 0, whiteMaterial);
			}
			else if(v < 0)
			{
				if(color != 4)
				{
					color = 4;		// RED for dropped snapshots
					Renderer.setColor(Color.Red);
				}
				Renderer.drawStretchPic(rect.x + rect.width - a, rect.y + rect.height - range, 1, range, 0, 0, 0, 0, whiteMaterial);
			}
		}

		Renderer.setColor(Color.White);

		/*
		if(cg_nopredict.integer || cg_synchronousClients.integer)
		{
			//CG_Drrect.widthBigString(rect.x, rect.y, "snc", 1.0);
		}
		*/

		renderDisconnect();
	}
}
